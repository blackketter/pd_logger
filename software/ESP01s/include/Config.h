#pragma once

// ==== Timing ====
static const unsigned long SAMPLE_INTERVAL_MS = 5000; // 5 Sekunden

// ==== I2C-Pins (ESP-01S) ====
static const int PIN_SDA = 2;  // GPIO2
static const int PIN_SCL = 0;  // GPIO0

// ==== Logging (konservativ für ESP-01S 1MB Flash) ====
// Zielgröße: ~64 KB Gesamt -> 4 Dateien à 16 KB
static const char* LOG_DIR    = "/logs";
static const char* LOG_PREFIX = "log_";      // log_0000.csv, log_0001.csv, ...
static const char* LOG_EXT    = ".csv";
static const size_t MAX_LOG_FILE_SIZE = 16 * 1024; // 16 KB pro Datei
static const size_t MAX_LOG_FILES     = 4;         // max. 4 Dateien (gesamt ~64 KB)

// ==== NTP / Zeitzone ====
static const char* TZ_EU_BERLIN = "CET-1CEST,M3.5.0,M10.5.0/3";

// ==== INA219-Korrektur (Adafruit-Referenz 100 mΩ vs. real 50 mΩ) ====
static const float SHUNT_MILLIOHM      = 50.0f;   // deine Platine
static const float INA219_REF_MILLIOHM = 100.0f;  // setCalibration_32V_2A() Annahme
static const float INA219_CORR = INA219_REF_MILLIOHM / SHUNT_MILLIOHM; // = 2.0f