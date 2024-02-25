// influxdb.go
package influxdb

import (
	"fmt" // Import fmt package
	"time"

	"backend/internal/types"
	"github.com/influxdata/influxdb1-client/v2"
	"os"
	"strconv"
)

// GetInfluxDBClient creates a new InfluxDB client
func GetInfluxDBClient() (client.Client, error) {
	// Required environment variables
	requiredEnvVars := []string{"INFLUXDB_ADDRESS", "INFLUXDB_PORT", "INFLUXDB_USERNAME", "INFLUXDB_PASSWORD", "INFLUXDB_DATABASE"}

	// Check if all required environment variables are set
	missingEnvVars := make([]string, 0)
	for _, envVar := range requiredEnvVars {
		if os.Getenv(envVar) == "" {
			missingEnvVars = append(missingEnvVars, envVar)
		}
	}

	// If any required environment variable is missing, return an error
	if len(missingEnvVars) > 0 {
		return nil, fmt.Errorf("Please ensure that the following environment variables are defined: %s", missingEnvVars)
	}

	// Resolve InfluxDB connection details from environment variables
	addr := os.Getenv("INFLUXDB_ADDRESS")
	port := os.Getenv("INFLUXDB_PORT")
	username := os.Getenv("INFLUXDB_USERNAME")
	password := os.Getenv("INFLUXDB_PASSWORD")

	// Create InfluxDB client
	return client.NewHTTPClient(client.HTTPConfig{
		Addr:     "http://" + addr + ":" + port,
		Username: username,
		Password: password,
	})
}

// writeToInfluxDB writes sensor data to InfluxDB
func WriteToInfluxDB(sensorData types.SensorData) error {
	// Get InfluxDB client
	influxDBClient, err := GetInfluxDBClient()
	if err != nil {
		return err
	}
	defer influxDBClient.Close()

	// Create a new point batch
	bp, err := client.NewBatchPoints(client.BatchPointsConfig{
		Database:  os.Getenv("INFLUXDB_DATABASE"),
		Precision: "s",
	})
	if err != nil {
		return err
	}

	// Create a new point
	tags := map[string]string{"id": strconv.Itoa(sensorData.ID)}
	fields := make(map[string]interface{})
	for key, value := range sensorData.Sensors {
		fields[key] = value
	}
	pt, err := client.NewPoint("sensors", tags, fields, time.Now())
	if err != nil {
		return err
	}
	bp.AddPoint(pt)

	// Write the batch to InfluxDB
	if err := influxDBClient.Write(bp); err != nil {
		return err
	}

	return nil
}
