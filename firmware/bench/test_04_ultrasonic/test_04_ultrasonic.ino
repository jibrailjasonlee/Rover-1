// hold your hand at 10 cm then 30 cm, readings should roughly match.
// echo goes through the 1k/2k divider, never straight into the pin.

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
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 30 ms timeout is about 5 m
  unsigned long us = pulseIn(ECHO_PIN, HIGH, 30000);

  if (us == 0) {
    Serial.println("no echo");
  } else {
    // round trip, hence the /2
    Serial.print(us * 0.0343 / 2);
    Serial.println(" cm");
  }
  delay(200);
}
