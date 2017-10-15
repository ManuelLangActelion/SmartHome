#include <DummyChildSensor.h>

DummyChildSensor::DummyChildSensor(uint8_t sensorId, uint8_t sensorType, uint8_t sensorPin)
{
  sensor = sensorId;
  type = sensorType;
  pin = sensorPin;
}

void DummyChildSensor::setup() {

}

void DummyChildSensor::present() {

}

bool DummyChildSensor::reportValue() {
  return true;
}

uint8_t DummyChildSensor::getValue() {
  return 12;
}

bool DummyChildSensor::hasNewValue() {
  return true;
}
