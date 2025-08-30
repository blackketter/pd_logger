#include <Arduino.h>
#include <Wire.h>
#include <SSD1306Wire.h>
#include <Adafruit_INA219.h>

// ===================
// Konfiguration
// ===================

// Display: I2C an D2(SDA), D1(SCL), 64x48 Pixel
SSD1306Wire display(0x3C, D2, D1, GEOMETRY_64_48);

// INA219-Sensor
Adafruit_INA219 ina219;

// Anzeige- und Timing-Parameter
constexpr uint8_t  DISPLAY_WIDTH      = 64;
constexpr uint16_t UPDATE_INTERVAL_MS = 500;

// Shunt-Anpassung: 100 mΩ Standard → 50 mΩ real → Faktor 2
constexpr float SHUNT_CORRECTION = 2.0f;

unsigned long lastUpdate = 0;

// ===================
// Hilfsfunktionen
// ===================

/**
 * Berechnet die X-Position für zentrierten Text innerhalb der Displaybreite.
 */
static inline int centeredX(const String &text) {
  const int pixelWidth = display.getStringWidth(text);
  const int x = (DISPLAY_WIDTH - pixelWidth) / 2;
  return (x < 0) ? 0 : x;
}

/**
 * Zeichnet einen String zentriert auf gegebener Y-Position.
 */
static inline void drawCentered(int y, const String &text) {
  display.drawString(centeredX(text), y, text);
}

/**
 * Zeigt eine einzeilige Statusmeldung zentriert an.
 */
static inline void showStatus(const String &msg) {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  drawCentered(12, msg);
  display.display();
}

// ===================
// Messwertanzeige
// ===================

void showMeasurements() {
  // Messwerte erfassen
  const float busVoltage_V    = ina219.getBusVoltage_V();    // VBUS
  const float shuntVoltage_mV = ina219.getShuntVoltage_mV(); // Shunt-Spannung (nur gelesen)
  float current_mA            = ina219.getCurrent_mA();
  float power_mW              = ina219.getPower_mW();

  // Shunt-Korrektur anwenden (50 mΩ statt 100 mΩ)
  current_mA *= SHUNT_CORRECTION;
  power_mW   *= SHUNT_CORRECTION;

  // Displayausgabe
  display.clear();
  display.setFont(ArialMT_Plain_16);

  String busVoltageStr = String(busVoltage_V, 2) + " V";
  drawCentered(0, busVoltageStr);

  String currentStr = String(current_mA, 0) + " mA";
  drawCentered(16, currentStr);

  String powerStr = String(power_mW / 1000.0f, 2) + " W";
  drawCentered(32, powerStr);

  display.display();

  // USB-Serial parallel ausgeben
  Serial.print("U = ");
  Serial.print(busVoltage_V, 2);
  Serial.print(" V   I = ");
  Serial.print(current_mA, 1);
  Serial.print(" mA   P = ");
  Serial.print(power_mW / 1000.0, 2);
  Serial.println(" W");

  (void)shuntVoltage_mV; // bewusst nicht verwendet
}

// ===================
// Arduino-Setup/Loop
// ===================

void setup() {
  Serial.begin(115200);

  // Auf verfügbare Serial warten (max. 1,5 s)
  const unsigned long t0 = millis();
  while (!Serial && (millis() - t0 < 1500)) { /* warten */ }
  Serial.println("Systemstart...");

  delay(1000);

  Wire.begin();

  // OLED initialisieren
  display.init();
  display.flipScreenVertically();  // richtige Ausrichtung
  display.clear();
  display.display();

  delay(500);

  // INA219 initialisieren und Status anzeigen
  if (!ina219.begin()) {
    showStatus("INA219 FAIL");
  } else {
    showStatus("INA219 OK");
  }
}

void loop() {
  const unsigned long now = millis();
  if (now - lastUpdate >= UPDATE_INTERVAL_MS) {
    lastUpdate = now;
    showMeasurements();
  }
}