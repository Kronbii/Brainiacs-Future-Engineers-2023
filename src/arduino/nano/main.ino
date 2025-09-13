/*
 * Arduino Nano Color Detection (Refactored)
 * Responsibilities:
 *  - Read TCS34725 sensor
 *  - Determine orange / blue / unknown
 *  - Output digital signals for Mega consumption (pins 9 & 10 as in legacy)
 */
#include <Wire.h>
#include <Adafruit_TCS34725.h>

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// Threshold constants (tunable)
const int ORANGE_MIN_RED = 20;
const int ORANGE_MAX_GREEN = 150;
const int ORANGE_MAX_BLUE = 90;

const int BLUE_MIN_BLUE = 11; // ensure >10
const int BLUE_MAX_RED = 24;  // ensure <25
const uint16_t MAX_CLEAR = 5200;

const uint8_t PIN_OUT_ORANGE = 9;
const uint8_t PIN_OUT_BLUE = 10;

struct ColorSample { uint16_t r,g,b,c; };

ColorSample readColor() {
  ColorSample s{};
  tcs.setInterrupt(false);
  tcs.getRawData(&s.r, &s.g, &s.b, &s.c);
  tcs.setInterrupt(true);
  return s;
}

bool isOrange(const ColorSample &s) {
  return (s.r > ORANGE_MIN_RED && s.g < ORANGE_MAX_GREEN && s.b < ORANGE_MAX_BLUE && s.r > s.b && s.r > s.g);
}

bool isBlue(const ColorSample &s) {
  return (s.b > s.r && s.b > s.g && s.c < MAX_CLEAR && s.b >= BLUE_MIN_BLUE && s.r <= BLUE_MAX_RED);
}

void publish(const char *label) {
  Serial.println(label);
}

void setup() {
  Serial.begin(9600);
  pinMode(PIN_OUT_ORANGE, OUTPUT);
  pinMode(PIN_OUT_BLUE, OUTPUT);
  if (!tcs.begin()) {
    publish("ERR:No TCS34725 detected");
    while (1) { delay(1000); }
  }
  publish("OK:SensorReady");
}

void loop() {
  auto sample = readColor();
  Serial.print(F("R:")); Serial.print(sample.r);
  Serial.print(F(" G:")); Serial.print(sample.g);
  Serial.print(F(" B:")); Serial.print(sample.b);
  Serial.print(F(" C:")); Serial.print(sample.c);

  bool orange = isOrange(sample);
  bool blue = !orange && isBlue(sample); // mutually exclusive prioritizing orange

  if (orange) {
    digitalWrite(PIN_OUT_ORANGE, HIGH);
    digitalWrite(PIN_OUT_BLUE, LOW);
    Serial.println(F(" -> ORANGE"));
  } else if (blue) {
    digitalWrite(PIN_OUT_ORANGE, LOW);
    digitalWrite(PIN_OUT_BLUE, HIGH);
    Serial.println(F(" -> BLUE"));
  } else {
    digitalWrite(PIN_OUT_ORANGE, LOW);
    digitalWrite(PIN_OUT_BLUE, LOW);
    Serial.println(F(" -> UNKNOWN"));
  }
  delay(50); // modest loop pace
}
