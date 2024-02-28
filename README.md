# homemonitor
Monitoring humidity and temperature with grafana+influxdb+natsio backend


# Setup

## Setup for oracle cloud

1. Create new instance in oracle cloud
2. setup .ssh/config with ssh key for the instance
3. create inventory.yml with correct instance information based on your ssh/config
4. run `ansible-playbook -i inventory.yml frontend.yml`
5. setup ingress rules for your oracle cloud instance with following information
   * Source type: CIDR
   * Source CIDR: 0.0.0.0/0
   * Source port Range: leave empty
   * Destination port range: 3000 (default grafana port, modify docker-compose.yml if you want to use different port)
   * Description: Grafana

## Grafana

You first want to change the grafana default admin password.

1. ssh to your machine where you're hosting the docker-compose file
2. Run `docker-compose exec grafana /bin/bash`
3. Run `grafana-cli admin reset-admin-password <your-new-password>`

## Setup influxdb datasource

1. ssh to your machine where you're hosting the docker-compose file
2. Run `docker-compose exec influxdb influx`
3. Create new database `CREATE NEW DATABASE <name_of_databas>`
4. Login to grafana with grafana admin credentials
5. Go to datasources -> new datasource -> influxdb
   * Name: you choose
   * HTTP - url: http://influxdb:8086
   * Skip TLS verify: true
   * Database - name: `<name_of_database>`


# HW clients

The service doesn't care about your client as long as it can send correct json to nats.io broker.

*Json schema:*

```
{
   id: <int>,
   sensors: {
      <measurement_name>: <measurement_value>,
      <measurement_name n+1>: <measurement_value n+1>
   }
}
```

*Example json:*

```
{
   id: 1,
   sensors: {
      humidity: 47,
      temperature: 22
   }
}
```

## ESP8266 - Arduino

* Requirements:
   * Arduino IDE
   * DHT Sensor library: https://github.com/adafruit/DHT-sensor-library
   * Nats-io library: https://github.com/isobit/arduino-nats/tree/master
   * ArduinoJson library: https://arduinojson.org/

To use existing code from _client/esp8266/dht11/dht11.ino_ please create secrets.h in same _client/esp8266/dht11/_ folder.

Contents of the secrets.h

```
const char* ssid = "<your_wifi_ssid>";
const char* password = "<your_wifi_password>";
const char* nats_address = "<nats_io_broker_dns_or_ip>";
const char* nats_subject = "sensors";
```
