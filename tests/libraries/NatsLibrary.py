import json
from pynats import NATSClient

class NatsLibrary():
    def __init__(self, addr="localhost", port=4222):
        self._nats_addr = f"nats://{addr}:{port}"

    def set_subject(self, subject):
        self.subject = subject

    def publish_message(self, payload, subject=None):
        if subject is not None:
            self.subject = subject
        with NATSClient(url=self._nats_addr) as nc:
            nc.connect()
            nc.publish(subject=self.subject, payload=payload)
    
    def convert_to_json_string(self, dict_data):
        return json.dumps(dict_data)