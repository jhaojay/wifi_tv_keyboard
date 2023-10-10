# SPDX-FileCopyrightText: 2021 Kattni Rembor for Adafruit Industries
#
# SPDX-License-Identifier: MIT

"""Example for Pico. Turns on the built-in LED."""
import board
import digitalio

led = digitalio.DigitalInOut(board.LED)
led.direction = digitalio.Direction.OUTPUT

def led_on():
    led.value = True

def led_off():
    led.value = False



# SPDX-FileCopyrightText: 2022 Liz Clark for Adafruit Industries
#
# SPDX-License-Identifier: MIT

import os
import ipaddress
import wifi
import socketpool

print()
print("Connecting to WiFi")

#  connect to your SSID
wifi.radio.connect('4250f5', '5846-lime-39')

print("Connected to WiFi")

pool = socketpool.SocketPool(wifi.radio)

#  prints MAC address to REPL
print("My MAC addr:", [hex(i) for i in wifi.radio.mac_address])

#  prints IP address to REPL
print("My IP address is", wifi.radio.ipv4_address)



import board
import digitalio
import adafruit_minimqtt.adafruit_minimqtt as MQTT
import time


def message_callback(client, topic, message):
    print("Received:", (topic, message))
    if message == 'on':
        print("Turning on LED...")
        led_on()
    elif message == 'off':
        print("Turning off LED...")
        led_off()

# Set up the MQTT client
broker_address = '192.168.1.201'
mqtt_client = MQTT.MQTT(
    broker=broker_address,
    client_id="PicoWSubscriber",
    socket_pool=pool  # Link the socket pool
)


mqtt_client.on_message = message_callback

def connect_mqtt():
    while True:
        try:
            mqtt_client.connect()
            mqtt_client.subscribe("pico/led")
            print("Subscribed to topic: pico/led")
            break
        except (MQTT.MMQTTException, OSError) as e:
            print(e)
            print("Failed to connect, retrying...")
            time.sleep(5)

connect_mqtt()

try:
    while True:
        mqtt_client.loop()  # Replaces client.wait_msg()
except KeyboardInterrupt:
    mqtt_client.disconnect()
    print("Disconnected from MQTT broker.")



