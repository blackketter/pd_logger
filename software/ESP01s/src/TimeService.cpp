#include "TimeService.h"

void TimeService::begin(const char* tz) {
  setenv("TZ", tz, 1);
  tzset();
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

time_t TimeService::nowEpoch() const {
  time_t now = time(nullptr);
  if (now < 100000) return 0;
  return now;
}

bool TimeService::isSynced() const {
  return nowEpoch() != 0;
}

String TimeService::nowISO8601Local() const {
  time_t now = nowEpoch();
  if (now == 0) return String("1970-01-01T00:00:00+00:00");

  struct tm info;
#ifdef ESP8266
  localtime_r(&now, &info);
#else
  info = *localtime(&now);
#endif
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", &info);
  String s(buf);
  if (s.length() >= 5) {
    s = s.substring(0, s.length()-2) + ":" + s.substring(s.length()-2);
  }
  return s;
}
