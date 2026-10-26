// turn each wheel by hand, motors off. counts should move and go up for forward.
// counting down: swap that encoder's A and B. send 'r' over serial to zero.
// turn a wheel exactly one revolution to get the real ticks per rev.
// zeros or garbage: the encoder outputs may want 10k pull-ups to 3.3 V,
// pins 34-39 don't have internal ones.

#define ENC_L_A 34
#define ENC_L_B 35
#define ENC_R_A 36
#define ENC_R_B 39

volatile long countL = 0;
volatile long countR = 0;

void IRAM_ATTR onLeftA()  { if (digitalRead(ENC_L_B)) countL++; else countL--; }
void IRAM_ATTR onRightA() { if (digitalRead(ENC_R_B)) countR++; else countR--; }

void setup() {
  Serial.begin(115200);
  pinMode(ENC_L_A, INPUT); pinMode(ENC_L_B, INPUT);
  pinMode(ENC_R_A, INPUT); pinMode(ENC_R_B, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENC_L_A), onLeftA, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_R_A), onRightA, RISING);
  Serial.println("Bench test 3: turn each wheel by hand");
}

void loop() {
  if (Serial.available() && Serial.read() == 'r') {
    noInterrupts(); countL = 0; countR = 0; interrupts();
    Serial.println("counts reset");
  }
  static unsigned long last = 0;
  if (millis() - last >= 250) {
    last = millis();
    Serial.print("L: "); Serial.print(countL);
    Serial.print("   R: "); Serial.println(countR);
  }
}
