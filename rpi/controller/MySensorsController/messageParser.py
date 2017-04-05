from messageParserPresentation import *
from messageParserSetReq import *
from messageParserInternal import *
from myMessage import *

# https://www.mysensors.org/download/serial_api_20

# node-id/child-sensor-id/command/ack/type qos payload

# mysensors-out/254/1/1/0/16 0 1
# mysensors-out/177/255/0/0/17 0 2.1.1
# mysensors-out/177/255/3/0/6 0 0
# mysensors-out/177/255/3/0/11 0 MockMySensors
# mysensors-out/177/255/3/0/12 0 v0.5
# mysensors-out/177/22/0/0/24 0 Dust Level
# mysensors-out/177/22/1/0/37 0 12
# mysensors-out/177/19/0/0/20 0 Univeral Command
# mysensors-out/177/19/1/0/32 0 0
# mysensors-out/177/19/1/0/33 0 0


class MessageParser:

    presentation = "presentation"
    set = "set"
    req = "req"
    internal = "internal"
    stream = "stream"

    options = {
        0 : presentation,
        1 : set,
        2 : req,
        3 : internal,
        4 : stream
    }

    def parseMessage(self, message):
        command = int(message.command)
        commandName = self.options[command]
        message.command = commandName
        if commandName == MessageParser.presentation:
            message.messageType = MessageParserPresentation().parseMessage(message.messageType)
        elif commandName == MessageParser.set:
            message.messageType = MessageParserSetReq().parseMessage(message.messageType)
        elif commandName == MessageParser.req:
            message.messageType = MessageParserSetReq().parseMessage(message.messageType)
        elif commandName == MessageParser.internal:
            message.messageType = MessageParserInternal().parseMessage(message.messageType)
        elif commandName == MessageParser.stream:
            message.messageType = ''
        return  message

    def parseFromMqtt(self, messageString):
        msgTokens = messageString.split(' ')

        msgPath = msgTokens[0]

        message = MyMessage()
        message.payload = messageString.replace(msgPath + " ", "", 1)

        msgFields = msgPath.split("/")
        message.nodeId = msgFields[0]
        message.childSensorId = msgFields[1]
        message.command = msgFields[2]
        message.ack = msgFields[3]
        message.messageType = msgFields[4]

        # print(message.toString())
        message = self.parseMessage(message)
        print(message.toString())

    def buildMessageReq(self, nodeId, childSensorId, messageType):
        message = MyMessage()
        message.nodeId = nodeId
        message.childSensorId = childSensorId
        message.command = MessageParser.req
        message.messageType = messageType
        messagePath = "mysensors-in/" + message.nodeId + "/" + message.childSensorId + "/" + message.command + "/1/" + message.messageType
        return messagePath

    # Here is how to interpret the alphanumeric codes at the end of the sent/received messages. For example: s=31,c=1,t=16,pt=1,l=1,sg=0:1
    # s = sensor id
    # c = message type 0-4: presentation, set, req, internal or stream. See link
    # t = value type: S_VALUE during presentation or V_VALUE during set/req
    # pt = payload type: string, byte int, etc. See link
    # l = message length
    # sg = signed or unsigned message: 0 or 1 for false or true

    # TSP:MSG:READ 0-0-122 s=7,c=2,t=2,pt=0,l=2,sg=0:75

    # mosquitto_pub -h 127.0.0.1 -t mysensors-in/122/24/2/1/40 -m "ff0000"
    def buildMessageSet(self, nodeId, childSensorId, messageType, payload):
        message = MyMessage()
        message.nodeId = nodeId
        message.childSensorId = childSensorId
        message.command = MessageParser.req
        message.messageType = messageType
        messagePath = "mysensors-in/" + str(message.nodeId) + "/" + str(message.childSensorId) + "/" + str(message.command) + "/1/" + str(message.messageType) + " " + str(payload)
        print(messagePath)
		#return messagePath
