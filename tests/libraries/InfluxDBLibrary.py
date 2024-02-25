from robot.api import logger
from influxdb import InfluxDBClient
class InfluxDBLibrary():
    influxdb_username=None
    influxdb_password=None
    influxdb_database=None
    def __init__(self, addr="localhost", port=8086):
        self.influxdb_addr=addr
        self.influxdb_port = port
    
    def set_credentials(self, username, password):
        self.influxdb_username=username
        self.influxdb_password=password
    
    def use_database(self, db_name):
        self.influxdb_database = db_name
    
    def send_query(self, query):
        if query.startswith("INSERT") or query.startswith("DELETE"):
            raise ValueError(f"This keyword is meant to use with SELECT, not: {query}")
        client = InfluxDBClient(self.influxdb_addr,
                                self.influxdb_port,
                                self.influxdb_username,
                                self.influxdb_password,
                                self.influxdb_database)
        result = client.query(query)
        logger.info(result)
        result_data = {}
        for measurement, points in result.items():
            result_data[measurement] = []
            for point in points:
                result_data[measurement].append(point)
        return result_data