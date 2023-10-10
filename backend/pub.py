import paho.mqtt.client as mqtt

broker_address = "localhost"
topic = "pico/led"

client = mqtt.Client("RPi4Publisher")
client.connect(broker_address)

client.publish(topic, 'on')
# mosquitto_pub -h localhost -t pico/led -m on

