# Software & Firmware Architecture

## Layered Overview
```
+----------------------------------------------------+
|                    Jetson Nano                     |
|  Python: Vision pipeline (OpenCV), sign detection  |
+----------------------+-----------------------------+
                       | UART / USB serial
+----------------------+-----------------------------+
|                 Arduino Mega (C++)                 |
|  Motor control, steering PID, sensor fusion (IMU)  |
+----------------------+-----------------------------+
                       | I2C
+----------------------+-----------------------------+
|          Sensors: MPU6050, TCS34725 Color          |
+----------------------------------------------------+
```

## Arduino Firmware
Folders:
- `src/ArduinoMegaCodeV1.0` main control loop
- `src/ArduinoNanoCodeV1.0` auxiliary logic (extend description as needed)
- `src/Arduino Libraries` vendor / third‑party libraries snapshot

### Control Loop (Conceptual Pseudocode)
```
readIMU();
readColorSensor();
updateHeadingPID();
applyMotorOutputs();
processSerialCommandsFromJetson();
telemetryTick();
```

## Vision Stack (Jetson)
`src/Jetson Code/main.py` handles:
- Frame capture from Raspberry Pi camera
- Preprocessing & color space conversions
- Traffic sign detection (describe method: template match / ML model placeholder)
- Command generation: steering adjustment or event message to Arduino

## Communication Protocol
Simple ASCII or binary framed serial (document once protocol formalized):
- J->M: `CMD:STEER:<value>`
- M->J: `TEL:IMU:<yaw>,<pitch>,<roll>`

## Calibration Procedures
1. IMU: keep level, capture bias over N samples.
2. Color Sensor: sample track corner colors under lighting, store thresholds.
3. PID: start with conservative P, increment until oscillation then add D.

## Build / Run
Arduino: open `.ino` in Arduino IDE (board: Mega 2560, correct port) and upload.
Jetson: ensure Python deps (OpenCV, numpy). Run: `python3 main.py`.

## Future Software Enhancements
- Replace heuristic sign detection with lightweight CNN.
- Add config file + runtime tuning over serial.
- Implement watchdog & fault codes.
