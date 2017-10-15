#ifndef DummyChildSensor_h
#define DummyChildSensor_h

#include <AbtractChildSensor.h>

class DummyChildSensor : public AbtractChildSensor<uint8_t> {

public:
  // Constructors
  DummyChildSensor(uint8_t sensorId, uint8_t sensorType, uint8_t sensorPin);

  void setup();
  void present();
  bool reportValue();
  uint8_t getValue();
  bool hasNewValue();
}
