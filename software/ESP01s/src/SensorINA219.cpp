#include "SensorINA219.h"
#include "Config.h"
#include <math.h>

bool SensorINA219::begin(TwoWire& w) {
  _wire = &w;
  if (!_ina.begin(_wire)) {
    return false;
  }
  _ina.setCalibration_32V_2A();
  return true;
}


static bool finiteRange(float x, float lo, float hi) {
  return isfinite(x) && x >= lo && x <= hi;
}

bool SensorINA219::read(Measurement& m) {
  // bis zu 3 Versuche bei Ausfall
  for (int attempt = 0; attempt < 3; ++attempt) {
    float shuntmV = _ina.getShuntVoltage_mV();
    float busV    = _ina.getBusVoltage_V();
    float currmA  = _ina.getCurrent_mA();
    float powermW = _ina.getPower_mW();

    // Plausibilitätsgrenzen (nach Kalibrierung anpassen!)
    bool ok =
      finiteRange(busV,   0.0f, 26.0f) &&        // INA219 Bus max ~26 V
      finiteRange(currmA, -5000.f, 5000.f) &&    // ±5 A Spielraum
      finiteRange(shuntmV, -100.0f, 100.0f) &&   // ±100 mV über Shunt
      finiteRange(powermW, -20000.f, 20000.f);   // ±20 W Reserve

    if (ok) {
      m.shuntmV = shuntmV;
      m.busV    = busV;
      m.currmA  = currmA * _corr;   // Korrektur anwenden
      m.powermW = powermW * _corr;  // Leistung skaliert linear mit Strom
      m.loadV   = m.busV + (m.shuntmV / 1000.0f);
      return true;
    }

    // kurzer Yield + nächster Versuch
    yield();
  }
  return false; // Caller soll diese Messung nicht loggen
}