#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include "Measurement.h"

class SensorINA219 {
public:
  bool begin(TwoWire& w);
  bool read(Measurement& m);
private:
  Adafruit_INA219 _ina;
  TwoWire* _wire = nullptr;
};
