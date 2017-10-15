/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik Ekblad
 *
 * DESCRIPTION
 * Example sketch for a "relay switch" where you can control binary or something
 * else from both HA controller and a local physical button
 * (connected between digital pin 3 and GND).
 * This node also works as a repeader for other nodes
 * http://www.mysensors.org/build/relay
 */

// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

#include <SPI.h>
#include <MySensors.h>
#include <Bounce2.h>

#define RELAY_PIN  6  // Arduino Digital I/O pin number for relay
#define RADIO_PIN_IRQ 2
#define BUTTON_PIN  3  // Arduino Digital I/O pin number for button
#define CHILD_ID 1   // Id of the sensor child
#define RELAY_ON 0
#define RELAY_OFF 1

Bounce debouncer = Bounce();
int oldValue=0;

static unsigned long last_interrupt_time = 0;
static bool initialized = false;
MyMessage msg(CHILD_ID,V_STATUS);

void setup()
{
  // Setup the button
  pinMode(BUTTON_PIN,INPUT);
  // Activate internal pull-up
  digitalWrite(BUTTON_PIN,HIGH);

  // After setting up the button, setup debouncer
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);

  // Make sure relays are off when starting up
  digitalWrite(RELAY_PIN, RELAY_OFF);
  // Then set relay pins in output mode
  pinMode(RELAY_PIN, OUTPUT);

  // Set relay to last known state (using eeprom storage)
  setState(getState());

  //attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), processButton, FALLING);
}

void presentation()  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Relay & Button", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID, S_BINARY);
}

void processButton() {
  Serial.print("=== Interrupt");
  Serial.println(millis());
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if ((interrupt_time - last_interrupt_time) > 150)
  {
    if (digitalRead(BUTTON_PIN) == LOW) {
        setState(!getState());
    }
  }
  last_interrupt_time = interrupt_time;
}

bool getState() {
  return loadState(CHILD_ID);
}
void setState(bool newState) {
  int oldState = getState();
  if (oldState != newState || !initialized) {
    initialized = true;
    Serial.println(newState);
    digitalWrite(RELAY_PIN, newState?RELAY_ON:RELAY_OFF);
    // Store state in eeprom
    saveState(CHILD_ID, newState);
    send(msg.set(newState), true); // Send new state and request ack back
  }
}

/*
*  Example on how to asynchronously check for new messages from gw
*/

void loop()
{
  debouncer.update();
  // Get the update value
  int value = debouncer.read();
  if (value != oldValue) {
      Serial.print("value:");
      Serial.println(value);
      if (value == LOW) {
        setState(!getState());
      }
      oldValue = value;
  }
}

void receive(const MyMessage &message) {
  Serial.print("Incoming message from gateway:");
  Serial.println(message.getCommand());
  Serial.println(message.getInt());

  // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
     Serial.println("This is an ack from gateway");
  } else {
    if (message.type == V_STATUS) {
       // Change relay state
       setState(message.getBool());

       // Write some debug info
       Serial.print("Incoming change for sensor:");
       Serial.print(message.sensor);
       Serial.print(", New status: ");
       Serial.println(message.getBool());
     }
  }
}
