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
