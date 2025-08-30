#include "DataLogger.h"

static String joinPath(const String& dir, const String& file) {
  if (dir.endsWith("/")) return dir + file;
  return dir + "/" + file;
}

bool DataLogger::ensureDir() const {
  if (LittleFS.exists(_dir)) return true;
  return LittleFS.mkdir(_dir);
}

bool DataLogger::parseIndex(const String& name, const String& prefix, const String& ext, int& out) {
  if (!name.startsWith(prefix) || !name.endsWith(ext)) return false;
  const int start = prefix.length();
  const int end   = name.length() - ext.length(); // ext ist bereits String
  if (end - start != 4) return false;             // genau 4 Ziffern
  for (int i = start; i < end; ++i) {
    if (name[i] < '0' || name[i] > '9') return false;
  }
  out = name.substring(start, end).toInt();
  return true;
}

String DataLogger::makeName(const String& prefix, int index, const String& ext) {
  char buf[16];
  snprintf(buf, sizeof(buf), "%04d", index & 0xFFFF);
  return prefix + String(buf) + ext;
}

void DataLogger::scanExisting(int& minIdx, int& maxIdx, size_t& count) const {
  minIdx =  0x7FFFFFFF;
  maxIdx = -0x7FFFFFFF;
  count  = 0;

  Dir dir = LittleFS.openDir(_dir);
  while (dir.next()) {
    String fname = dir.fileName();
    int idx;
    if (parseIndex(fname, _prefix, _ext, idx)) {
      count++;
      if (idx < minIdx) minIdx = idx;
      if (idx > maxIdx) maxIdx = idx;
    }
  }
  if (count == 0) {
    minIdx = maxIdx = -1;
  }
}

bool DataLogger::createNewFile(int index) {
  const String fname = makeName(_prefix, index, _ext);
  _currentPath  = joinPath(_dir, fname);
  File f = LittleFS.open(_currentPath, "w");
  if (!f) return false;
  // Kompakter Header: nur epoch;bus_V;curr_mA
  f.println(F("epoch;bus_V;curr_mA"));
  f.close();
  _currentIndex = index;
  return true;
}

bool DataLogger::begin(const char* dirPath, const char* prefix, const char* ext,
                       size_t maxFileSize, size_t maxFiles) {
  _dir = dirPath;
  _prefix = prefix;
  _ext = ext;
  _maxFileSize = maxFileSize;
  _maxFiles = maxFiles;

  if (!ensureDir()) return false;

  int minIdx, maxIdx;
  size_t count;
  scanExisting(minIdx, maxIdx, count);

  if (count == 0) {
    // erste Datei
    return createNewFile(0);
  } else {
    _currentIndex = maxIdx;
    _currentPath  = joinPath(_dir, makeName(_prefix, _currentIndex, _ext));
    if (!LittleFS.exists(_currentPath)) {
      // Sicherheitsnetz, falls die ermittelte Datei fehlt
      return createNewFile(maxIdx >= 0 ? maxIdx + 1 : 0);
    }
    return true;
  }
}

bool DataLogger::append(const Measurement& m, const String&) {
  // epoch (Sekunden), Spannung in mV (int), Strom in mA (int)
  int32_t epoch   = (m.epoch > 0) ? (int32_t)m.epoch : 0;
  int32_t bus_mV  = (int32_t)lroundf(m.busV   * 1000.0f);
  int32_t curr_mA = (int32_t)lroundf(m.currmA);

  File f = LittleFS.open(_currentPath, "a");
  if (!f) return false;

  f.print(epoch);   f.print(';');
  f.print(bus_mV);  f.print(';');   // mV (integer)
  f.println(curr_mA);               // mA (integer)
  f.close();

  return rotateIfNeeded();
}

bool DataLogger::rotateIfNeeded() {
  File f = LittleFS.open(_currentPath, "r");
  size_t size = f ? f.size() : 0;
  if (f) f.close();
  if (size < _maxFileSize) return true;

  int minIdx, maxIdx;
  size_t count;
  scanExisting(minIdx, maxIdx, count);

  const int nextIdx = (count == 0) ? 0 : (maxIdx + 1);

  if (count >= _maxFiles && minIdx >= 0) {
    const String oldestPath = joinPath(_dir, makeName(_prefix, minIdx, _ext));
    LittleFS.remove(oldestPath);
    // count reduziert sich implizit; wir brauchen es nicht weiter
  }

  return createNewFile(nextIdx);
}

size_t DataLogger::listFilesJSON(String& outJson) const {
  struct Item { int idx; String name; size_t size; };
  Item items[64];
  size_t n = 0;

  Dir dir = LittleFS.openDir(_dir);
  while (dir.next()) {
    yield(); // WDT füttern während Dir-Iteration
    const String fname = dir.fileName();
    int idx;
    if (parseIndex(fname, _prefix, _ext, idx)) {
      if (n < 64) {
        items[n].idx  = idx;
        items[n].name = (_dir.endsWith("/") ? _dir : _dir + "/") + fname;
        File f = LittleFS.open(items[n].name, "r");
        items[n].size = f ? f.size() : 0;
        if (f) f.close();
        n++;
        yield(); // WDT nach File-Open/Close
      }
    }
  }

  // einfache Insertion-Sort nach idx
  for (size_t i = 1; i < n; ++i) {
    Item key = items[i];
    size_t j = i;
    while (j > 0 && items[j-1].idx > key.idx) { items[j] = items[j-1]; j--; }
    items[j] = key;
  }

  String json = "[";
  for (size_t i = 0; i < n; ++i) {
    if (i) json += ",";
    json += "{\"name\":\"" + items[i].name + "\",\"size\":" + String(items[i].size) + "}";
  }
  json += "]";
  outJson = json;
  return n;
}

bool DataLogger::clearAll() {
  if (!ensureDir()) return false;

  // alle Dateien im Log-Verzeichnis löschen
  Dir dir = LittleFS.openDir(_dir);
  while (dir.next()) {
    yield();
    String p = dir.fileName();
    if (!p.startsWith("/")) p = "/" + p;
    if (!p.startsWith(_dir + "/")) {
      int slash = p.lastIndexOf('/');
      p = _dir + "/" + (slash >= 0 ? p.substring(slash + 1) : p);
    }
    LittleFS.remove(p);
  }

  // internen Zustand zurücksetzen (optional, begin setzt ohnehin neu)
  _currentIndex = -1;
  _currentPath  = String();

  // frisch initialisieren – begin legt "log_0000.csv" an und schreibt den Header
  return begin(_dir.c_str(), _prefix.c_str(), _ext.c_str(), _maxFileSize, _maxFiles);
}