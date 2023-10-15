1. Using ESP32S3 since it has OTG and be able to act as HID.
   
2. Before running, please set your wifi, pass, and MQTT server IP.

3. And run idf.py menuconfig -> Component config -> TinyUSB Stack -> Human Interface Device Class (HID) -> TinyUSB HID interfaces count -> 1.

