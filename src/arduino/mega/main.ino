/*
 * Arduino Mega Main Control (Refactored)
 * Responsibilities:
 *  - IMU based heading PID when Jetson inactive
 *  - Corner / color based turning logic
 *  - Jetson override of steering via serial (angle command)
 */
#include <Wire.h>
#include <MPU6050_tockn.h>
#include <Servo.h>

// --- Hardware Pins ---
const uint8_t PIN_SERVO = 5;
const uint8_t PIN_MOTOR_PWM = 4; // A in original code
const uint8_t PIN_ORANGE = 10;   // sensor digital
const uint8_t PIN_BLUE = 11;     // sensor digital
const uint8_t PIN_JETSON_MODE = 7; // 0 => local PID, 1 => Jetson steer

const uint8_t PIN_MOTOR_DIR_FWD = 3; // MP
const uint8_t PIN_MOTOR_DIR_REV = 2; // MN

// --- PID Config ---
double Kp = 1.33;
double Ki = 0.0;
double Kd = 2.0;

// --- State ---
MPU6050 mpu(Wire);
Servo steering;

long lastError = 0;
long integral = 0;
double headingSetpoint = 0.0; // Target yaw angle

int8_t cornerOrangeLatched = 0;
int8_t cornerBlueLatched = 0;

// Jetson command
int jetsonAngle = 0; // -30..30 expected

// --- Utility Functions ---
long currentHeading() {
  return mpu.getAngleZ();
}

int clampInt(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

int pidStep() {
  long yaw = currentHeading();
  long error = headingSetpoint - yaw;
  long d = error - lastError;
  integral += error;
  long output = (long)(Kp * error + Kd * d + Ki * integral);
  lastError = error;
  // Limit output to servo range influence
  if (output > 30) output = 30;
  if (output < -30) output = -30;
  return (int)output;
}

void applySteering(int angleOffset) {
  // Servo center empirically ~80 in original code.
  steering.write(80 - angleOffset); // negative due to original orientation choice
}

void maybeHandleCorners(long yaw) {
  int orange = digitalRead(PIN_ORANGE);
  int blue = digitalRead(PIN_BLUE);

  // Orange turn: subtract 85 degrees
  if (orange == HIGH && !cornerOrangeLatched) {
    while (digitalRead(PIN_BLUE) == LOW || (currentHeading() - headingSetpoint) > -85) {
      applySteering(-30); // steer right (?) based on original sign usage
      mpu.update();
    }
    cornerBlueLatched = 1; // prevent immediate opposite trigger
    headingSetpoint -= 85;
    applySteering(30);
  }
  // Blue turn: add 85 degrees
  else if (blue == HIGH && !cornerBlueLatched) {
    while (digitalRead(PIN_ORANGE) == LOW || (currentHeading() - headingSetpoint) < 85) {
      applySteering(30);
      mpu.update();
    }
    cornerOrangeLatched = 1;
    headingSetpoint += 85;
    applySteering(-30);
  }
}

void maybeReadJetson() {
  if (Serial.available()) {
    int val = Serial.parseInt();
    jetsonAngle = clampInt(val, -30, 30);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(PIN_ORANGE, INPUT);
  pinMode(PIN_BLUE, INPUT);
  pinMode(PIN_JETSON_MODE, INPUT);
  pinMode(PIN_MOTOR_DIR_FWD, OUTPUT);
  pinMode(PIN_MOTOR_DIR_REV, OUTPUT);
  pinMode(PIN_MOTOR_PWM, OUTPUT);

  Wire.begin();
  mpu.begin();
  mpu.calcGyroOffsets(true);

  steering.attach(PIN_SERVO);
  analogWrite(PIN_MOTOR_PWM, 145); // maintain original throttle baseline
}

void loop() {
  mpu.update();
  int jetsonMode = digitalRead(PIN_JETSON_MODE); // 0 => PID, 1 => Jetson

  if (jetsonMode == 0) {
    int pidOut = pidStep();
    applySteering(pidOut);
    maybeHandleCorners(currentHeading());
  } else {
    maybeReadJetson();
    applySteering(jetsonAngle);
  }

  // Basic telemetry (could be expanded / throttled)
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 200) {
    lastPrint = millis();
    Serial.print(F("HDG:")); Serial.print(currentHeading());
    Serial.print(F(",SP:")); Serial.print(headingSetpoint);
    Serial.print(F(",JET:")); Serial.print(jetsonAngle);
    Serial.println();
  }
}
