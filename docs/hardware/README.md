# Hardware Overview

## Bill of Materials (Core Components)
| Component | Qty | Purpose |
|-----------|-----|---------|
| NVIDIA Jetson Nano | 1 | Runs computer vision & sign detection |
| Arduino Mega 2560 | 1 | Main motion & sensor controller |
| Arduino Nano | 1 | Auxiliary tasks / modular sensor handling |
| Raspberry Pi Camera v2.1 | 1 | Vision input for Jetson |
| Brushless DC Motor | 1 | Drive propulsion |
| Geared Servo Motor | 1 | Steering control |
| L298N Dual H-Bridge | 1 | Motor driver |
| MPU6050 IMU | 1 | Orientation & PID feedback |
| TCS34725 Color Sensor | 1 | Corner & marker detection |
| 18650 Li-Ion Cells | 5 | Power source |

(Add part numbers + links if allowed.)

## Electrical Architecture
See `schemes/` for the full wiring schematic. Key buses:
- I2C: MPU6050, TCS34725
- PWM: Servo output from Mega
- UART / USB: Jetson ↔ Mega (command & telemetry)

## Power Notes
- Separate regulated rails recommended: logic 5V vs motor supply.
- Common ground between Jetson, Arduinos, and motor driver.

## Mechanical / CAD
3D printable models reside in `models/`. Provide assembly order:
1. Mount Jetson carrier.
2. Install drive motor & gear interface.
3. Attach steering servo + linkage.
4. Fix sensor mast (camera + color + IMU).

## Future Hardware Improvements
- Add wheel encoders for closed-loop velocity.
- Shift to a more efficient motor driver (MOSFET based).
- Integrate fused distribution board.
