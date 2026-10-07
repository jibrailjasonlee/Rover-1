// ============================================================
// Rover Firmware — Skeleton v1 (Structure & Pin Setup)
// ============================================================
// This is a LIVING file — we'll come back and fill in each
// stubbed-out function over the next several lessons:
//   driveMotors()     <- PWM + direction control
//   readEncoders()    <- interrupt-based pulse counting
//   readDistance()    <- ultrasonic sensor
//   updatePID()        <- closed-loop speed correction
//   handleWebRequests() <- WiFi teleop
// ============================================================

// --- #define vs const: a quick note ---
// #define is a PREPROCESSOR directive: before your code is even
// compiled, the compiler does a dumb find-and-replace, swapping
// every "MOTOR_L_PWM" in your file with the literal text "14".
// It has no type and takes zero memory at runtime.
// This is the traditional Arduino convention for pin numbers,
// so we'll use it here — but know that modern C++ generally
// prefers `const uint8_t` instead (a typed, safer alternative).
// You'll see both conventions in real embedded code.

// --- Motor driver (TB6612FNG) pins ---
#define STBY        27  // Standby pin — driver ignores inputs until this is HIGH

#define MOTOR_L_IN1 26  // Direction control pin 1, left motor
#define MOTOR_L_IN2 25  // Direction control pin 2, left motor
#define MOTOR_L_PWM 14  // Speed control (PWM), left motor

#define MOTOR_R_IN1 33  // Direction control pin 1, right motor
#define MOTOR_R_IN2 32  // Direction control pin 2, right motor
#define MOTOR_R_PWM 13  // Speed control (PWM), right motor

// --- Encoder pins ---
// These are ESP32's INPUT-ONLY pins (34-39). They physically
// cannot drive an output signal, only read one — which is all
// an encoder pulse needs, so this is the correct pin family
// to use here, not an arbitrary choice.
#define ENC_L_A     34
#define ENC_L_B     35
#define ENC_R_A     36
#define ENC_R_B     39

// --- Ultrasonic sensor (HC-SR04) ---
#define TRIG_PIN    5
#define ECHO_PIN    18  // Reminder: needs a voltage divider —
                         // this pin outputs 5V from the sensor,
                         // but ESP32 GPIOs are only 3.3V-tolerant.

// --- Encoder pulse counters ---
// `volatile` tells the compiler: "this variable can change at
// any moment, outside the normal flow of the code — don't
// optimize based on assumptions about when it changes."
// This matters because interrupt service routines (which we'll
// write in the next lesson) modify these variables asynchronously,
// completely outside your normal loop() execution.
// `long` (not `int`) because these counters need to hold large
// values over a long drive without overflowing — an int on the
// ESP32 is 32-bit same as long here, but `long` signals intent
// clearly to anyone reading the code later.
volatile long encoderCountLeft = 0;
volatile long encoderCountRight = 0;

// ============================================================
// PID controller — closed-loop speed correction
// ============================================================
// Why this is needed: command both motors to the same PWM value
// and they will NOT spin at the same actual speed. Manufacturing
// variance, friction differences, even which side the battery
// wire runs closer to — all cause small mismatches that add up
// to the robot slowly curving instead of driving straight.
// PID continuously compares ACTUAL speed (from the encoders) to
// TARGET speed and corrects the PWM output to close that gap.
//
// A `struct` groups related variables into one named bundle.
// Here, each motor needs its OWN independent PID state (its own
// accumulated error, its own last-error value) — rather than
// writing out separate kp_left, ki_left, integral_left, kp_right...
// as a pile of loose variables, we define the shape ONCE as a
// struct, then create two instances of it.
//
// IMPORTANT (this is the fix for the compile error you just hit):
// this struct is defined here, ABOVE the first function in the
// file (the ISRs below). The Arduino IDE auto-generates forward
// declarations for every function and inserts them all at the
// very top of the file before compiling — if a function's
// signature uses PIDController and the struct itself is defined
// further down the file than that insertion point, the generated
// declaration references a type the compiler doesn't know about
// yet. Keeping custom structs/classes above your first function
// sidesteps this entirely.
struct PIDController {
  float kp;              // Proportional gain — reacts to the CURRENT error
  float ki;              // Integral gain — reacts to ACCUMULATED past error
  float kd;              // Derivative gain — reacts to how fast error is CHANGING
  float integral = 0;    // Running total of error over time
  float lastError = 0;   // Error from the previous update, for derivative math
};

// `float` is a new type here: unlike `int` or `long`, it holds
// decimal values — essential for PID math, where gains like 0.5
// or 2.3 are completely normal and an integer type couldn't
// represent them at all.
//
// These starting gain values are reasonable PLACEHOLDERS, not
// tuned numbers — real PID tuning is an empirical process you do
// with the physical robot: start with only Kp nonzero, increase
// it until the robot responds quickly without oscillating, then
// add a small Ki to eliminate any steady drift, then a small Kd
// if it overshoots. We'll do this properly once hardware arrives.
PIDController pidLeft  = {2.0, 0.5, 0.1};
PIDController pidRight = {2.0, 0.5, 0.1};

// Note the `&` before PIDController — this passes the struct BY
// REFERENCE, not by value. Without it, this function would receive
// a COPY of pidLeft/pidRight, and any changes to .integral or
// .lastError inside this function would vanish the moment it
// returns — the whole point of tracking accumulated error over
// time would silently break. `&` ensures we're modifying the
// actual original struct, not a throwaway copy.
float computePID(PIDController &pid, float setpoint, float actual, float dt) {
  float error = setpoint - actual;

  pid.integral += error * dt;
  float derivative = (error - pid.lastError) / dt;
  pid.lastError = error;

  return (pid.kp * error) + (pid.ki * pid.integral) + (pid.kd * derivative);
}

// ============================================================
// Interrupt Service Routines (ISRs) — encoder pulse counting
// ============================================================
// Polling (checking a pin's state inside loop(), over and over)
// would work, but it's unreliable here: your encoder can spin
// fast enough to produce pulses shorter than one pass through
// loop(), especially once loop() also has WiFi and PID work in
// it — you'd simply miss pulses, silently, with no error.
//
// An INTERRUPT solves this differently: instead of you asking
// "has the pin changed?" repeatedly, the hardware itself pauses
// whatever loop() is doing the INSTANT the pin changes, jumps to
// run this small function, then resumes loop() exactly where it
// left off. Nothing gets missed, regardless of what loop() was
// busy doing.
//
// `IRAM_ATTR` is ESP32-specific (you won't see this on classic
// AVR Arduino boards): it forces this function to be stored in
// fast internal RAM rather than flash memory. This matters because
// flash access can briefly pause during certain background
// operations — an ISR living in flash could then run at the
// worst possible moment. IRAM_ATTR guarantees it's always
// immediately available.
//
// This is QUADRATURE decoding: the encoder has two output
// channels (A and B), slightly out of phase with each other.
// By triggering on channel A's rising edge and then checking
// channel B's CURRENT state at that instant, we can tell not
// just THAT the motor turned, but which DIRECTION — B's state
// at the moment A rises is different depending on rotation
// direction.
void IRAM_ATTR onLeftEncoderA() {
  if (digitalRead(ENC_L_B)) {
    encoderCountLeft++;
  } else {
    encoderCountLeft--;
  }
}

void IRAM_ATTR onRightEncoderA() {
  if (digitalRead(ENC_R_B)) {
    encoderCountRight++;
  } else {
    encoderCountRight--;
  }
}

// ============================================================
// setup() — runs ONCE when the board powers on or resets.
// This is where you configure pin modes, start serial
// communication, and do any one-time initialization.
// ============================================================
void setup() {
  // Serial.begin starts communication over USB so you can print
  // debug messages to your computer. 115200 is the "baud rate" —
  // how fast data is sent, in bits per second. This number just
  // has to match on both ends (your code and the Serial Monitor).
  Serial.begin(115200);

  // pinMode(pin, mode) tells the chip whether a pin will be used
  // to send voltage out (OUTPUT) or read voltage in (INPUT).
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

  // attachInterrupt(pin, function, triggerMode) registers an ISR.
  // digitalPinToInterrupt() converts a plain GPIO number into the
  // specific interrupt channel the hardware needs — always wrap
  // your pin number in this, don't pass the raw number directly.
  // RISING means "trigger when this pin transitions LOW -> HIGH."
  attachInterrupt(digitalPinToInterrupt(ENC_L_A), onLeftEncoderA, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_R_A), onRightEncoderA, RISING);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // The TB6612FNG ignores all motor inputs while STBY is LOW —
  // it's a power-saving standby mode. We have to explicitly wake
  // it up before any motor command will have an effect.
  digitalWrite(STBY, HIGH);

  Serial.println("Rover firmware initialized.");
}

// ============================================================
// loop() — runs REPEATEDLY, forever, after setup() finishes.
// This is your main program. Note what's NOT here: no delay().
// We'll talk about why in the next lesson — short version, a
// robot juggling motors, sensors, and WiFi can't afford to
// freeze execution while waiting on a timer.
// ============================================================
void loop() {
  // driveMotors() is gone now — updatePID() has taken over motor
  // control, since speed is no longer a fixed hardcoded value but
  // something continuously corrected against real encoder feedback.
  updatePID();
  readDistance();

  // --- Non-blocking periodic debug print ---
  // We want to print encoder counts every 500ms, WITHOUT using
  // delay(500) — delay() would freeze the whole chip, including
  // motor updates and (eventually) WiFi handling, for that entire
  // half second. Every time, on every loop. That's not acceptable
  // for a robot doing several things concurrently.
  //
  // Instead: compare the current time (millis(), which counts
  // milliseconds since boot) against the last time we printed.
  // If enough time has passed, print and update our "last time."
  // This way loop() keeps running at full speed continuously —
  // it just conditionally does the print, instead of stopping
  // everything to wait.
  //
  // `static` here means this variable keeps its value between
  // calls to loop() — normally a local variable resets every time
  // a function runs, but `static` makes it persist, which is
  // exactly what we need to remember "when did I last print?"
  static unsigned long lastPrintTime = 0;
  unsigned long now = millis();

  if (now - lastPrintTime >= 500) {
    lastPrintTime = now;
    Serial.print("Encoders -> L: ");
    Serial.print(encoderCountLeft);
    Serial.print("  R: ");
    Serial.println(encoderCountRight);
  }
}

// ============================================================
// Function stubs — each gets a real implementation in a
// future lesson. For now, they just exist so the file compiles
// and the overall structure is visible.
// ============================================================

// --- Helper function: controls ONE motor ---
// Rather than writing near-identical direction+speed logic twice
// (once per motor), we write it ONCE as a function that takes the
// specific pins as PARAMETERS — then call it twice with different
// arguments. This is a core idea in good code organization: if
// you're about to copy-paste a block and change only a few values,
// that's usually a sign it should be a function instead.
//
// `int speed` is deliberately SIGNED (can be negative), unlike the
// uint8_t we used for pin numbers — here negative means "reverse,"
// so the sign itself carries meaning, which requires a signed type.
void setMotorSpeed(uint8_t in1, uint8_t in2, uint8_t pwmPin, int speed) {
  // constrain() is a built-in Arduino function: it clamps a value
  // to a min/max range. If speed came in as 400, this forces it
  // down to 255 instead of sending an invalid PWM value.
  speed = constrain(speed, -255, 255);

  // The TB6612FNG reads IN1/IN2 as a direction code:
  //   HIGH, LOW  -> forward
  //   LOW, HIGH  -> reverse
  //   LOW, LOW   -> coast (no braking force, freewheels)
  if (speed > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }

  // PWM (Pulse Width Modulation) is how we control speed with only
  // an on/off digital pin: instead of a true analog voltage, the
  // pin switches HIGH/LOW very fast (thousands of times a second),
  // and the PERCENTAGE of time it spends HIGH (the "duty cycle")
  // is what the motor actually feels as an average voltage.
  // analogWrite() takes a duty cycle from 0 (always off) to 255
  // (always on) and handles the fast switching for you.
  // abs() is another built-in — it strips the sign, since PWM duty
  // cycle is always a positive magnitude; direction is handled
  // separately above by IN1/IN2, not by the PWM value itself.
  analogWrite(pwmPin, abs(speed));
}

// --- State needed to measure actual speed from raw encoder counts ---
long lastEncoderLeft = 0;
long lastEncoderRight = 0;

void updatePID() {
  // Same non-blocking timing pattern from the encoder lesson —
  // run this block on a fixed ~50ms cadence rather than every
  // single pass through loop() (which happens far more often
  // than that). Too fast and the encoder delta between updates
  // is tiny and noisy; too slow and the robot reacts sluggishly.
  static unsigned long lastTime = 0;
  unsigned long now = millis();
  unsigned long dtMs = now - lastTime;
  if (dtMs < 50) return;

  float dt = dtMs / 1000.0;  // milliseconds -> seconds, for the math below
  lastTime = now;

  // How many encoder pulses accumulated since the last update?
  long deltaLeft = encoderCountLeft - lastEncoderLeft;
  long deltaRight = encoderCountRight - lastEncoderRight;
  lastEncoderLeft = encoderCountLeft;
  lastEncoderRight = encoderCountRight;

  // Convert raw pulse counts into a speed: pulses per second.
  // (Once you know your encoder's pulses-per-revolution from the
  // datasheet, this can be converted into real RPM — for now,
  // pulses/sec works fine as a relative speed measure for PID.)
  float actualSpeedLeft  = deltaLeft  / dt;
  float actualSpeedRight = deltaRight / dt;

  // Placeholder target — will come from WiFi commands in the
  // teleop lesson, instead of being hardcoded here.
  float targetSpeed = 300.0;

  float outputLeft  = computePID(pidLeft,  targetSpeed, actualSpeedLeft,  dt);
  float outputRight = computePID(pidRight, targetSpeed, actualSpeedRight, dt);

  // (int) explicitly CONVERTS the float PID output into an int —
  // C++ won't do this silently in a function call like this one,
  // you have to ask for it. setMotorSpeed() still clamps to
  // ±255 internally via constrain(), so this is a safe handoff
  // even if the PID math briefly produces a wild value.
  setMotorSpeed(MOTOR_L_IN1, MOTOR_L_IN2, MOTOR_L_PWM, (int)outputLeft);
  setMotorSpeed(MOTOR_R_IN1, MOTOR_R_IN2, MOTOR_R_PWM, (int)outputRight);
}

void readDistance() {
  // TODO: trigger the ultrasonic sensor and measure echo time
}
