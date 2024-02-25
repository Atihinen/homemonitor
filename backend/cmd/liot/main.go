package main

import (
	"backend/internal/influxdb"
	"backend/internal/types"
	"encoding/json"
	"fmt"
	"github.com/nats-io/nats.go"
	"log"
	"os"
	"os/signal"
	"strconv"
	"syscall"
)

func main() {
	// Connect to NATS server
	nc, err := nats.Connect(nats.DefaultURL)
	if err != nil {
		log.Fatalf("Error connecting to NATS: %v", err)
	}
	defer nc.Close()
	log.Printf("Connected to nats")
	// Subscribe to "sensors" topic
	_, err = nc.Subscribe("sensors", func(msg *nats.Msg) {
		log.Printf("Received message: %s", msg.Data)
		unescapedJSON, err := strconv.Unquote(string(msg.Data))
		log.Printf("Unescaped json: %s", unescapedJSON)
		if err != nil {
			fmt.Println("Error unescaping JSON:", err)
			return
		}
		var sensorData types.SensorData
		// Unmarshal the unescaped JSON into sensorData
		err = json.Unmarshal([]byte(unescapedJSON), &sensorData)
		if err != nil {
			log.Printf("Error decoding JSON: %v", err)
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
