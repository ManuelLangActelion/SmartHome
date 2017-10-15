#ifndef AbtractChildSensor_h
#define AbtractChildSensor_h

#include <MyMessage.h>
#include <vector>

template <class T>
class AbtractChildSensor {
private:
  vector<T> values;
  uint8_t type;            	 // 8 bit - Type varies depending on command
  uint8_t sensor;
  uint8_t pin;

public:
	// Constructors
	AbtractChildSensor(uint8_t sensorId, uint8_t sensorType, uint8_t sensorPin);

  virtual void setup();
  virtual void present();
  virtual bool reportValue();
  virtual T getValue();
  virtual bool hasNewValue();
}
