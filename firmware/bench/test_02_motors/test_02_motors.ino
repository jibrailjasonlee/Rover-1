// Bench test 2: do both motors turn, in both directions?
// Needs: ESP32 on USB, TB6612FNG wired, motor rail on (6 V buck),
//        all grounds shared. WHEELS OFF THE GROUND (or removed).
// PASS: each motor turns forward, then backward, one at a time.
// If a motor spins the wrong way: swap that motor's two wires.
// If a motor hums but does not turn: raise TEST_PWM (try 120).

#define STBY        27
#define MOTOR_L_IN1 26
#define MOTOR_L_IN2 25
#define MOTOR_L_PWM 14
#define MOTOR_R_IN1 33
#define MOTOR_R_IN2 32
#define MOTOR_R_PWM 13

const int TEST_PWM = 80;   // out of 255: gentle on purpose

void setMotor(uint8_t in1, uint8_t in2, uint8_t pwmPin, int speed) {
  speed = constrain(speed, -255, 255);
  if (speed > 0)      { digitalWrite(in1, HIGH); digitalWrite(in2, LOW); }
  else if (speed < 0) { digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); }
  else                { digitalWrite(in1, LOW);  digitalWrite(in2, LOW); }
  analogWrite(pwmPin, abs(speed));
}

void runMotor(const char* label, uint8_t in1, uint8_t in2, uint8_t pwmPin, int speed) {
  Serial.println(label);
  setMotor(in1, in2, pwmPin, speed);
  delay(1500);
  setMotor(in1, in2, pwmPin, 0);   // always stop before the next step
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  pinMode(STBY, OUTPUT);
  pinMode(MOTOR_L_IN1, OUTPUT); pinMode(MOTOR_L_IN2, OUTPUT); pinMode(MOTOR_L_PWM, OUTPUT);
  pinMode(MOTOR_R_IN1, OUTPUT); pinMode(MOTOR_R_IN2, OUTPUT); pinMode(MOTOR_R_PWM, OUTPUT);
  digitalWrite(STBY, HIGH);
  Serial.println("Bench test 2: WHEELS OFF THE GROUND. Starting in 5 s...");
  delay(5000);
}

void loop() {
  runMotor("left forward",  MOTOR_L_IN1, MOTOR_L_IN2, MOTOR_L_PWM,  TEST_PWM);
  runMotor("left backward", MOTOR_L_IN1, MOTOR_L_IN2, MOTOR_L_PWM, -TEST_PWM);
  runMotor("right forward", MOTOR_R_IN1, MOTOR_R_IN2, MOTOR_R_PWM,  TEST_PWM);
  runMotor("right backward",MOTOR_R_IN1, MOTOR_R_IN2, MOTOR_R_PWM, -TEST_PWM);
  Serial.println("--- cycle done, pausing 3 s ---");
  delay(3000);
}
