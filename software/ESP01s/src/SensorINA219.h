#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include "Measurement.h"

class SensorINA219 {
public:
  bool begin(TwoWire& w);
  bool read(Measurement& m);
  void setShuntCorrection(float factor) { _corr = (isfinite(factor) && factor > 0) ? factor : 1.0f; }

private:
  Adafruit_INA219 _ina;
  TwoWire* _wire = nullptr;
  float _corr = 1.0f;
};