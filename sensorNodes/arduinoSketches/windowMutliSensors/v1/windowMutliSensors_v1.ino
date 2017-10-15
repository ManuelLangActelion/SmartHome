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
 * Version 1.0: Henrik EKblad
 * Version 1.1 - 2016-07-20: Converted to MySensors v2.0 and added various improvements - Torben Woltjen (mozzbozz)
 *
 * DESCRIPTION
 * This sketch provides an example of how to implement a humidity/temperature
 * sensor using a DHT11/DHT-22.
 *
 * For more information, please visit:
 * http://www.mysensors.org/build/humidity
 *
 */

// Enable debug prints
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69
//#define MY_RS485

#include <SPI.h>
#include <MySensors.h>
#include <DHT.h>
#include <Bounce2.h>


#define PIN_BATTERY_SENSE A0  // select the input pin for the battery sense point
#define PIN_TMP36 A6
#define PIN_LDR A7

#define PIN_SENSORS_INTERRUPT 3

#define PIN_MOTION_SENSOR  4
#define PIN_DHT_DATA 5
#define PIN_WINDOW_SWITCH  7  // Arduino Digital I/O pin for button/reed switch

// Set this offset if the sensor has a permanent small offset to the real temperatures
#define SENSOR_TEMP_OFFSET 0

#define ledPin 13


// Sleep time between sensor updates (in milliseconds)
// Must be >1000ms for DHT22 and >2000ms for DHT11
static const uint64_t SLEEP_TIME = 2500; // Sleep time between reports (in milliseconds)


// Force sending an update of the temperature after n sensor reads, so a controller showing the
// timestamp of the last update doesn't show something like 3 hours in the unlikely case, that
// the value didn't change since;
// i.e. the sensor would force sending an update every SLEEP_TIME*FORCE_UPDATE_N_READS [ms]
static const uint8_t FORCE_UPDATE_N_READS = 10;

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_WINDOW_SWITCH 2
#define CHILD_MOTION_SENSOR 3
#define CHILD_TEMP 4
#define CHILD_LIGHT 5

float lastTemp;
float lastHum;
uint8_t nNoUpdatesTemp;
uint8_t nNoUpdatesHum;
bool metric = true;

int windowSwitchOldValue=-1;
int oldBatteryPcnt = 0;
int motionOldValue = 0;
float tempOldValue = 0;
float ldrOldValue = 0;
int ledValue = 0;

MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgWindow(CHILD_WINDOW_SWITCH,V_TRIPPED);
MyMessage msgMotion(CHILD_MOTION_SENSOR, V_TRIPPED);
MyMessage msgTemp26(CHILD_TEMP, V_TEMP);
MyMessage msgLight(CHILD_LIGHT, V_LIGHT_LEVEL);

DHT dht;
Bounce debouncerWindow = Bounce();


void setup()
{
  pinMode (ledPin, OUTPUT);

  pinMode(PIN_WINDOW_SWITCH, INPUT_PULLUP);

  pinMode(PIN_MOTION_SENSOR, INPUT);      // sets the motion sensor digital pin as input

  // After setting up the button, setup debouncer
  debouncerWindow.attach(CHILD_WINDOW_SWITCH);
  debouncerWindow.interval(5);

  #if defined(__AVR_ATmega2560__)
    analogReference(INTERNAL1V1); // use the 1.1 V internal reference
  #else
    analogReference(INTERNAL);
  #endif

  dht.setup(PIN_DHT_DATA); // set data pin of DHT sensor
  if (SLEEP_TIME <= dht.getMinimumSamplingPeriod()) {
    Serial.println("Warning: SLEEP_TIME is smaller than supported by the sensor!");
  }
  // Sleep for the time of the minimum sampling period to give the sensor time to power up
  // (otherwise, timeout errors might occure for the first reading)
  sleep(dht.getMinimumSamplingPeriod());
}


void presentation()
{
  // Send the sketch version information to the gateway
  sendSketchInfo("MultiSensors", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_HUM, S_HUM);
  present(CHILD_ID_TEMP, S_TEMP);
  present(CHILD_WINDOW_SWITCH, S_DOOR);
  present(CHILD_MOTION_SENSOR, S_MOTION);
  present(CHILD_TEMP, S_TEMP);
  present(CHILD_LIGHT, S_LIGHT_LEVEL);

  metric = getControllerConfig().isMetric;
}


void loop()
{
  // get the battery Voltage
  int sensorValue = analogRead(PIN_BATTERY_SENSE);
#ifdef MY_DEBUG
  Serial.println(sensorValue);
#endif
  // 1M, 470K divider across battery and using internal ADC ref of 1.1V
  // Sense point is bypassed with 0.1 uF cap to reduce noise at that point
  // ((1e6+470e3)/470e3)*1.1 = Vmax = 3.44 Volts
  // 3.44/1023 = Volts per bit = 0.003363075

  int batteryPcnt = sensorValue / 10;

#ifdef MY_DEBUG
    float batteryV  = sensorValue * 0.003363075;
    Serial.print("Battery Voltage: ");
    Serial.print(batteryV);
    Serial.println(" V");

    Serial.print("Battery percent: ");
    Serial.print(batteryPcnt);
    Serial.println(" %");
#endif
  if (oldBatteryPcnt != batteryPcnt) {
      // Power up radio after sleep
      sendBatteryLevel(batteryPcnt);
      oldBatteryPcnt = batteryPcnt;
  }


  debouncerWindow.update();
  // Get the update value
  int valueWindow = debouncerWindow.read();
  if (valueWindow != windowSwitchOldValue) {
     send(msgWindow.set(valueWindow == HIGH ? 1 : 0));
     windowSwitchOldValue = valueWindow;
  }

  bool motionTripped = digitalRead(PIN_MOTION_SENSOR) == HIGH;
#ifdef MY_DEBUG
  Serial.println(motionTripped);
#endif

  if (motionTripped != motionOldValue) {
    send(msgMotion.set(motionTripped ? 1 : 0));
    motionOldValue = motionTripped;
  }

  // Force reading sensor, so it works also after sleep()
  dht.readSensor(true);

  // Get temperature from DHT library
  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed reading temperature from DHT!");
  } else if (temperature != lastTemp || nNoUpdatesTemp == FORCE_UPDATE_N_READS) {
    // Only send temperature if it changed since the last measurement or if we didn't send an update for n times
    lastTemp = temperature;
    if (!metric) {
      temperature = dht.toFahrenheit(temperature);
    }
    // Reset no updates counter
    nNoUpdatesTemp = 0;
    temperature += SENSOR_TEMP_OFFSET;
    send(msgTemp.set(temperature, 1));

    #ifdef MY_DEBUG
    Serial.print("T: ");
    Serial.println(temperature);
    #endif
  } else {
    // Increase no update counter if the temperature stayed the same
    nNoUpdatesTemp++;
  }

  // Get humidity from DHT library
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
    Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum || nNoUpdatesHum == FORCE_UPDATE_N_READS) {
    // Only send humidity if it changed since the last measurement or if we didn't send an update for n times
    lastHum = humidity;
    // Reset no updates counter
    nNoUpdatesHum = 0;
    send(msgHum.set(humidity, 1));

    #ifdef MY_DEBUG
    Serial.print("H: ");
    Serial.println(humidity);
    #endif
  } else {
    // Increase no update counter if the humidity stayed the same
    nNoUpdatesHum++;
  }

  float tmp36Value = analogRead(PIN_TMP36);
#ifdef MY_DEBUG
  Serial.println(tmp36Value);
#endif
  if (tmp36Value != tempOldValue) {
    send(msgTemp26.set(tmp36Value, 1));
    tempOldValue = tmp36Value;
  }

  float lightValue = analogRead(PIN_LDR);
#ifdef MY_DEBUG
  Serial.println(lightValue);
#endif
  if (lightValue != ldrOldValue) {
    send(msgLight.set(lightValue, 1));
    ldrOldValue = lightValue;
  }

  ledValue = (valueWindow || motionTripped) ? HIGH : LOW;
  digitalWrite(ledPin, ledValue);

  // Sleep until interrupt comes in or sleep time is over, to save battery
  sleep(digitalPinToInterrupt(PIN_SENSORS_INTERRUPT), CHANGE, SLEEP_TIME);
}
