#include <AbtractChildSensor.h>


vector<T> ver(0);

AbtractChildSensor::AbtractChildSensor(uint8_t sensorId, uint8_t sensorType, uint8_t sensorPin)
{
  sensor = sensorId;
  type = sensorType;
  pin = sensorPin;
}

virtual void AbtractChildSensor::setup();

virtual void AbtractChildSensor::present();

virtual bool ChildSensor::reportValue();

virtual T ChildSensor::getValue();

virtual bool ChildSensor::hasNewValue();
