#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include "Measurement.h"

class DataLogger {
public:
  // Initialisiert Logger (Rotation): z.B. dir="/logs", prefix="log_", ext=".csv"
  bool begin(const char* dirPath, const char* prefix, const char* ext,
             size_t maxFileSize, size_t maxFiles);

  // Schreibt einen Datensatz (epoch;bus_mV;curr_mA) und prüft ggf. Rotation
  bool append(const Measurement& m, const String& isoLocal /*unused*/);

  // Liefert JSON-Array mit {name,size} aller Log-Dateien (aufsteigend sortiert)
  size_t listFilesJSON(String& outJson) const;

  // Aktueller Dateipfad
  String currentFilePath() const { return _currentPath; }

  // Löscht alle Log-Dateien und startet frisch (begin(...) intern erneut aufgerufen)
  bool clearAll();

private:
  String _dir;
  String _prefix;
  String _ext;
  size_t _maxFileSize = 0;
  size_t _maxFiles = 0;
  String _currentPath;
  int _currentIndex = -1;

  bool ensureDir() const;
  void scanExisting(int& minIdx, int& maxIdx, size_t& count) const;
  static bool parseIndex(const String& name, const String& prefix, const String& ext, int& out);
  static String makeName(const String& prefix, int index, const String& ext);
  bool createNewFile(int index);
  bool rotateIfNeeded();
};