python3 ~/SmartHome/mqtt_exemple.py

mosquitto_pub -h 127.0.0.1 -p 1883 -u homeassistant -P Actel10n! -t "mysensors-in/99/1/1/1/3" -m 70

mosquitto_pub -h 127.0.0.1 -p 1883 -u homeassistant -P Actel10n! -t "mysensors-in/99/1/1/1/23" -m 1
mosquitto_pub -h 127.0.0.1 -p 1883 -u homeassistant -P Actel10n! -t "mysensors-in/99/1/1/1/23" -m 0

mosquitto_pub -h 127.0.0.1 -p 1883 -u homeassistant -P Actel10n! -t "office/sensor1" -m '{"temp":23,"hum":58}'