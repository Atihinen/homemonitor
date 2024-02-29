package main

import (
	"backend/internal/influxdb"
	"backend/internal/types"
	"encoding/json"
	"fmt"
	"log"
	"os"
	"os/signal"
	"strconv"
	"syscall"

	"github.com/nats-io/nats.go"
)

func main() {
	// Connect to NATS server
	natsAddress := os.Getenv("NATS_ADDRESS")
	if natsAddress == "" {
		log.Fatalf("Error: NATS_ADDRESS environment variable is not defined")
	}
	nc, err := nats.Connect(natsAddress)
	if err != nil {
		log.Fatalf("Error connecting to NATS: %v", err)
	}
	defer nc.Close()
	log.Printf("Connected to nats")
	// Subscribe to "sensors" topic
	_, err = nc.Subscribe("sensors", func(msg *nats.Msg) {
		log.Printf("Received message: %s", msg.Data)
		var sensorData types.SensorData
		if err := extractAndParseJSON(msg.Data, &sensorData); err != nil {
			log.Printf("Error extracting and parsing JSON: %v", err)
			return
		}
		// Process the received sensor data
		// Print all sensor values
		log.Printf("Received sensor data: ID=%d, Sensors:", sensorData.ID)
		for key, value := range sensorData.Sensors {
			log.Printf("%s: %v", key, value)
		}
		err = influxdb.WriteToInfluxDB(sensorData)
		if err != nil {
			log.Printf("Error writing to InfluxDB: %v", err)
			return
		}
	})
	if err != nil {
		log.Fatalf("Error subscribing to NATS topic: %v", err)
	}

	// Wait for interrupt signal to gracefully shutdown
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)
	<-sigChan
	log.Println("Shutting down...")
}

func extractAndParseJSON(data []byte, sensorData *types.SensorData) error {
	var unescapedJSON string

	// Check if the JSON is escaped or not
	if len(data) >= 2 && data[0] == '"' && data[len(data)-1] == '"' {
		// If the JSON is escaped, unquote it
		unescaped, err := strconv.Unquote(string(data))
		if err != nil {
			return fmt.Errorf("error unescaping JSON: %v", err)
		}
		unescapedJSON = unescaped
	} else {
		// If the JSON is not escaped, use it directly
		unescapedJSON = string(data)
	}

	// Unmarshal the unescaped JSON into sensorData
	if err := json.Unmarshal([]byte(unescapedJSON), sensorData); err != nil {
		return fmt.Errorf("error decoding JSON: %v", err)
	}

	return nil
}
