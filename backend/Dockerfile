FROM alpine:3.19.1
ENV NATS_ADDRESS=nats://localhost:4222
ENV INFLUXDB_ADDRESS=http://localhost
ENV INFLUXDB_PORT=8086
ENV INFLUXDB_USERNAME=user
ENV INFLUXDB_PASSWORD=pw
ENV INFLUXDB_DATABASE=test
COPY bin/liot /usr/local/bin/liot
RUN chmod u+x /usr/local/bin/liot
CMD /usr/local/bin/liot