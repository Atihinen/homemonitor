*** Settings ***
Library    NatsLibrary    ${NATS_ADDR}    ${NATS_PORT}
Library    InfluxDBLibrary    ${INFLUXDB_ADDR}    ${INFLUXDB_PORT}
Library    Collections

*** Variables ***
${PAYLOAD}=    {"id": 3, "sensors": {"humidity": 58, "temperature": 20}}

*** Test Cases ***

Send Sensor Payload
    [Setup]    Setup Influxdb Connection
    ${payload}=    Convert To Json String    ${PAYLOAD}
    Publish Message    ${payload}    sensors
    ${result}=    Send Query    SELECT * FROM sensors LIMIT 1
    Log    ${result}
    ${humidity}=    Read Sensor Value    ${result}    humidity
    Should Be Equal As Numbers    ${humidity}    47.0
    ${temperature}=    Read Sensor Value    ${result}    temperature
    Should Be Equal As Numbers    ${temperature}    19.0

*** Keywords ***
Setup Influxdb Connection
    Set Credentials    ${INFLUXDB_USERNAME}    ${INFLUXDB_PASSWORD}
    Use Database    ${INFLUXDB_DATABASE}

Read Sensor Value
    [Arguments]    ${query_result}    ${sensor}
    ${keys}=    Get Dictionary Keys    ${query_result}
    ${sensors_data}=   Get From Dictionary    ${query_result}    ${keys[0]} 
    ${sensor_data}=   Get From Dictionary    ${sensors_data[0]}    ${sensor}
    [RETURN]    ${sensor_data}
