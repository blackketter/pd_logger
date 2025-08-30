#pragma once
#include <Arduino.h>
#include <time.h>

struct Measurement {
  time_t epoch = 0;
  uint32_t ms = 0;
  float busV = 0;
  float shuntmV = 0;
  float currmA = 0;
  float powermW = 0;
  float loadV = 0;
};
