// Bench test 4: does the ultrasonic sensor read sensible distances?
// Needs: ESP32 on USB, HC-SR04 powered from 5 V, Echo through the
//        1k + 2k voltage divider (NEVER straight into the ESP32 pin).
// PASS: hold your hand 10 cm, then 30 cm in front of it; readings
//       roughly match. "no echo" means nothing was in range or a wiring problem.

#define TRIG_PIN 5
#define ECHO_PIN 18

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("Bench test 4: ultrasonic");
}

void loop() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);   // 10 us trigger pulse
  digitalWrite(TRIG_PIN, LOW);

  // pulseIn waits for the Echo pin to go HIGH, then times how long it
  // stays HIGH. 30000 us is a timeout, about 5 m of range.
  unsigned long us = pulseIn(ECHO_PIN, HIGH, 30000);

  if (us == 0) {
    Serial.println("no echo");
  } else {
    // Sound travels about 0.0343 cm per microsecond; the pulse covers
    // the trip out AND back, so divide by 2.
    Serial.print(us * 0.0343 / 2);
    Serial.println(" cm");
  }
  delay(200);
}
