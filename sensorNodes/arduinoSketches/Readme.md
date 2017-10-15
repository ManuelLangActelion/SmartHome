Arduino sketches for sensors


Relay and button:
mosquitto_pub -h 127.0.0.1 -p 1883 -u homeassistant -P **** -t "mysensors-in/99/1/1/1/23" -m 0
mosquitto_pub -h 127.0.0.1 -p 1883 -u homeassistant -P **** -t "mysensors-in/99/1/1/1/23" -m 1