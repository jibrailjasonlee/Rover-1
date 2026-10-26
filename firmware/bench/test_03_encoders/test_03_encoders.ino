// Bench test 3: do the encoders count, and in the right direction?
// Needs: ESP32 on USB, encoder wires connected, encoders powered from 3.3 V.
// Turn each wheel BY HAND (motors off).
// PASS: counts change when you turn a wheel, and "forward" gives a
//       positive count. If forward counts down, swap that encoder's A and B wires.
// Type r in the Serial Monitor and press Enter to zero both counts.
// BONUS: mark the wheel, turn it exactly one full revolution, and read
//        the count. That number is your real ticks-per-revolution.
// If counts are zero or erratic, the encoder outputs may need 10 kohm
// pull-up resistors to 3.3 V (pins 34-39 have no internal pull-ups).

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
