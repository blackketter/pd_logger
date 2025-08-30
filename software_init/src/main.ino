void setup() {
  Serial.begin(115200);      // Start USB CDC
  while (!Serial) {
    ; // warten, bis der Host verbunden ist (nur beim Start)
  }
  Serial.println("USB CDC ist aktiv!");
}

void loop() {
  Serial.println("Hallo vom STM32 - USB nur für Serial-Ausgaben!");
  delay(1000);
}