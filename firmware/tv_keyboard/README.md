Using ESP32S3 since it has OTG and be able to act as HID.
Before running, please set your wifi, pass, and MQTT server IP.
And run idf.py menuconfig -> Component config -> TinyUSB Stack -> Human Interface Device Class (HID) -> TinyUSB HID interfaces count -> 1.

