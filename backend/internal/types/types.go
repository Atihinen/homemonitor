// types.go
package types

// SensorData type definition
type SensorData struct {
	ID      int                    `json:"id"`
	Sensors map[string]interface{} `json:"sensors"`
}
