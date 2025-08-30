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

/*
Optional: dynamische Anpassung in setup(), falls du den Platz automatisch
an das tatsächliche LittleFS-Volume anpassen willst. Beispiel:

  FSInfo fs;
  LittleFS.info(fs);
  // nutze 60% des verfügbaren FS für Logs, aber clamp auf sinnvolle Grenzen
  size_t budget = (size_t)(fs.totalBytes * 0.60f);
  size_t fileSize = 16 * 1024; // Startwert
  size_t fileCount = max<size_t>(2, min<size_t>(10, budget / fileSize));

  // Dann DataLogger so initialisieren:
  logger.begin(LOG_DIR, LOG_PREFIX, LOG_EXT, fileSize, fileCount);

(Anmerkung: Für ESP-01S sind die obigen Konstanten i. d. R. schon passend.)
*/
