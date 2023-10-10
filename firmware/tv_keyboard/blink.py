import network
import secrets
from utime import sleep, ticks_ms, ticks_diff

print('Connecting to WiFi Network Name:', secrets.SSID)
wlan = network.WLAN(network.STA_IF)
wlan.active(True)

start = ticks_ms() # start a millisecond counter

if not wlan.isconnected():
    wlan.connect(secrets.SSID, secrets.PASSWORD)
    print("Waiting for connection...")
    counter = 0
    while not wlan.isconnected():
        sleep(1)
        print(counter, '.', sep='', end='', )
        counter += 1

delta = ticks_diff(ticks_ms(), start)
print("Connect Time:", delta, 'milliseconds')
print('IP Address:', wlan.ifconfig()[0])

from umqtt.simple import MQTTClient
from machine import Pin

def toggle_led():
    led = Pin("LED", Pin.OUT)
    led.toggle()
    

def message_callback(topic, msg):
    print("Received:", (topic, msg))
    if msg == b'toggle':
        print("Toggling LED...")
        toggle_led()

        
broker_address = '192.168.1.201'

client = MQTTClient("PicoWSubscriber", broker_address)
client.set_callback(message_callback)
def connect_mqtt():
    try:
        client.connect()
        client.subscribe("pico/led")
        print("Subscribed to topic: pico/led")

    except OSError as e:
        print(e)
        print("Failed to connect, retrying...")
        sleep(5)
        connect_mqtt()
        
connect_mqtt()

try:
    while True:
        client.wait_msg()
except KeyboardInterrupt:
    client.disconnect()
    print("Disconnected from MQTT broker.")



