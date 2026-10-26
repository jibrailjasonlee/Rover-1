#include <WiFi.h>
#include <WebServer.h>

#define STBY        27

#define MOTOR_L_IN1 26
#define MOTOR_L_IN2 25
#define MOTOR_L_PWM 14

#define MOTOR_R_IN1 33
#define MOTOR_R_IN2 32
#define MOTOR_R_PWM 13

// 34-39 are input only, which is all an encoder needs. no internal
// pull-ups on them though
#define ENC_L_A     34
#define ENC_L_B     35
#define ENC_R_A     36
#define ENC_R_B     39

#define TRIG_PIN    5
#define ECHO_PIN    18  // echo is 5 V, goes through a 1k/2k divider

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const float DRIVE_SPEED = 300.0;  // ticks/s
const float TURN_SPEED = 200.0;

volatile long encoderCountLeft = 0;
volatile long encoderCountRight = 0;

// has to sit above the first function. the IDE auto-generates prototypes
// at the top of the file and computePID's one breaks if it can't see this
struct PIDController {
  float kp;
  float ki;
  float kd;
  float integral = 0;
  float lastError = 0;
};

// placeholders. the sim in sim/ says these are far too high for the N20s
PIDController pidLeft  = {2.0, 0.5, 0.1};
PIDController pidRight = {2.0, 0.5, 0.1};

WebServer server(80);
float targetSpeedLeft = 0;
float targetSpeedRight = 0;
long lastEncoderLeft = 0;
long lastEncoderRight = 0;

float computePID(PIDController &pid, float setpoint, float actual, float dt) {
  float error = setpoint - actual;
  pid.integral += error * dt;  // no anti-windup yet
  float derivative = (error - pid.lastError) / dt;
  pid.lastError = error;
  return pid.kp * error + pid.ki * pid.integral + pid.kd * derivative;
}

// counts rising edges of A only, so one tick per pulse. B gives direction
void IRAM_ATTR onLeftEncoderA() {
  if (digitalRead(ENC_L_B)) encoderCountLeft++;
  else encoderCountLeft--;
}

void IRAM_ATTR onRightEncoderA() {
  if (digitalRead(ENC_R_B)) encoderCountRight++;
  else encoderCountRight--;
}

void handleRoot() {
  String html =
    "<!DOCTYPE html><html><body style='text-align:center;font-family:sans-serif;'>"
    "<h2>Rover Control</h2>"
    "<button onclick=\"fetch('/forward')\" style='font-size:24px;padding:20px;'>Forward</button><br><br>"
    "<button onclick=\"fetch('/left')\" style='font-size:24px;padding:20px;'>Left</button>"
    "<button onclick=\"fetch('/stop')\" style='font-size:24px;padding:20px;'>Stop</button>"
    "<button onclick=\"fetch('/right')\" style='font-size:24px;padding:20px;'>Right</button><br><br>"
    "<button onclick=\"fetch('/backward')\" style='font-size:24px;padding:20px;'>Backward</button>"
    "</body></html>";
  server.send(200, "text/html", html);
}

// TODO: nothing zeroes these if the phone drops, the rover keeps going
void setTargets(float left, float right) {
  targetSpeedLeft = left;
  targetSpeedRight = right;
  server.send(200, "text/plain", "OK");
}

void handleForward()  { setTargets(DRIVE_SPEED, DRIVE_SPEED); }
void handleBackward() { setTargets(-DRIVE_SPEED, -DRIVE_SPEED); }
void handleLeft()     { setTargets(-TURN_SPEED, TURN_SPEED); }
void handleRight()    { setTargets(TURN_SPEED, -TURN_SPEED); }
void handleStop()     { setTargets(0, 0); }

void setMotorSpeed(uint8_t in1, uint8_t in2, uint8_t pwmPin, int speed) {
  speed = constrain(speed, -255, 255);
  if (speed > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);  // coast, not brake
    digitalWrite(in2, LOW);
  }
  analogWrite(pwmPin, abs(speed));
}

void updatePID() {
  static unsigned long lastTime = 0;
  unsigned long now = millis();
  unsigned long dtMs = now - lastTime;
  // one tick in a 50 ms window is 20 ticks/s, shorter would be mostly noise
  if (dtMs < 50) return;

  // first call after boot has dt = millis() since lastTime starts at 0
  float dt = dtMs / 1000.0;
  lastTime = now;

  long deltaLeft = encoderCountLeft - lastEncoderLeft;
  long deltaRight = encoderCountRight - lastEncoderRight;
  lastEncoderLeft = encoderCountLeft;
  lastEncoderRight = encoderCountRight;

  float outputLeft  = computePID(pidLeft,  targetSpeedLeft,  deltaLeft / dt,  dt);
  float outputRight = computePID(pidRight, targetSpeedRight, deltaRight / dt, dt);

  setMotorSpeed(MOTOR_L_IN1, MOTOR_L_IN2, MOTOR_L_PWM, (int)outputLeft);
  setMotorSpeed(MOTOR_R_IN1, MOTOR_R_IN2, MOTOR_R_PWM, (int)outputRight);
}

void readDistance() {
  // TODO
}

void setup() {
  Serial.begin(115200);

  pinMode(STBY, OUTPUT);
  pinMode(MOTOR_L_IN1, OUTPUT);
  pinMode(MOTOR_L_IN2, OUTPUT);
  pinMode(MOTOR_L_PWM, OUTPUT);
  pinMode(MOTOR_R_IN1, OUTPUT);
  pinMode(MOTOR_R_IN2, OUTPUT);
  pinMode(MOTOR_R_PWM, OUTPUT);

  pinMode(ENC_L_A, INPUT);
  pinMode(ENC_L_B, INPUT);
  pinMode(ENC_R_A, INPUT);
  pinMode(ENC_R_B, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENC_L_A), onLeftEncoderA, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_R_A), onRightEncoderA, RISING);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(STBY, HIGH);  // driver ignores everything while this is low

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/backward", handleBackward);
  server.on("/left", handleLeft);
  server.on("/right", handleRight);
  server.on("/stop", handleStop);
  server.begin();
}

void loop() {
  server.handleClient();
  updatePID();
  readDistance();

  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime >= 500) {
    lastPrintTime = millis();
    Serial.print("L: ");
    Serial.print(encoderCountLeft);
    Serial.print("  R: ");
    Serial.println(encoderCountRight);
  }
}
