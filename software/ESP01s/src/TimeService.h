#pragma once
#include <Arduino.h>
#include <time.h>

class TimeService {
public:
  void begin(const char* tz);
  time_t nowEpoch() const;
  String nowISO8601Local() const;
  bool isSynced() const;
};
