// board alive check. nothing wired, just USB.
// pass: led blinks and serial monitor (115200) prints uptime

#define LED_PIN 2

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("Bench test 1: board is alive");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  Serial.print("uptime ms: ");
  Serial.println(millis());
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
