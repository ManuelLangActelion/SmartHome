import paho.mqtt.client as mqtt

# Define event callbacks
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("$SYS/#")

def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

def on_publish(mosq, obj, mid):
    print("mid: " + str(mid))

def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed: " + str(mid) + " " + str(granted_qos) + " " + str(obj) + " " + str(mosq))

def on_log(mosq, obj, level, string):
    print(string)

client = mqtt.Client()
# Assign event callbacks
client.on_message = on_message
client.on_connect = on_connect
client.on_publish = on_publish
client.on_subscribe = on_subscribe

# Uncomment to enable debug messages
#mqttc.on_log = on_log

# Parse CLOUDMQTT_URL (or fallback to localhost)
#url_str = os.environ.get('CLOUDMQTT_URL', 'mqtt://localhost:1883')
#url = urlparse.urlparse(url_str)

# Connect
# mqttc.username_pw_set(url.username, url.password)
client.connect("127.0.0.1", 1883, 60)

# Start subscribe, with QoS level 0
client.subscribe("mysensors-out/#", 0)

# Continue the network loop, exit when an error occurs
rc = 0
while rc == 0:
    rc = client.loop()
print("rc: " + str(rc))
