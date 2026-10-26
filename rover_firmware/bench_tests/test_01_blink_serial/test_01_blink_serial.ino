// Bench test 1: is the board alive, and can we talk to it?
// Needs: ESP32 on USB only. Nothing else connected.
// PASS: the onboard LED blinks and the Serial Monitor (115200 baud)
//       prints an uptime number every second.

#define LED_PIN 2   // onboard LED on most ESP32 DevKit boards

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("Bench test 1: board is alive");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  Serial.print("uptime ms: ");
  Serial.println(millis());
  delay(500);      // delay() is fine in tests; we only do one thing here
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
