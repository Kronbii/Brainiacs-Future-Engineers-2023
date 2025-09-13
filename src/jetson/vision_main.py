from __future__ import annotations
import cv2
import numpy as np
import serial
import time
from dataclasses import dataclass
from typing import Optional, Tuple

# ---------------- Configuration ---------------- #
KNOWN_HEIGHT_MM = 100

HSV_BLUE_RANGES = [
(np.array([40, 100, 100]), np.array([70, 255, 255])),
(np.array([70, 100, 100]), np.array([100, 255, 255])),
]
HSV_RED_RANGES = [
(np.array([0, 100, 100]), np.array([10, 255, 255])),
(np.array([160, 100, 100]), np.array([179, 255, 255])),
]

FX = 1430
FY = 1450
CX = 635
CY = 60

SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 9600

BLUE_ADJUST_DEG = -20
RED_ADJUST_DEG = 20
MAX_EFFECTIVE_DISTANCE_MM = 600
MIN_CONTOUR_AREA = 1000

# ------------------------------------------------ #

@dataclass
class Detection:
    color: str
    rect: Tuple[int, int, int, int]
    distance_mm: float
    angle_rad: float


def open_serial(port: str, baud: int) -> Optional[serial.Serial]:
    try:
        ser = serial.Serial(port, baud)
        time.sleep(2)
        return ser
    except Exception as e:  # pragma: no cover (hardware dependent)
        print(f"[WARN] Could not open serial: {e}")
        return None


def build_mask(hsv: np.ndarray, ranges) -> np.ndarray:
    mask_total = None
    for lower, upper in ranges:
        mask = cv2.inRange(hsv, lower, upper)
        mask_total = mask if mask_total is None else cv2.bitwise_or(mask_total, mask)
    kernel = np.ones((5, 5), np.uint8)
    mask_total = cv2.morphologyEx(mask_total, cv2.MORPH_CLOSE, kernel)
    mask_total = cv2.morphologyEx(mask_total, cv2.MORPH_OPEN, kernel)
    return mask_total


def distance_mm(perceived_h: int) -> float:
    return (KNOWN_HEIGHT_MM * FY) / perceived_h if perceived_h > 0 else 1e9


def horizontal_angle(perceived_x: float) -> float:
    return np.arctan((perceived_x - CX) / FX)


def closest_detection(contours, color: str) -> Optional[Detection]:
    best: Optional[Detection] = None
    for c in contours:
        area = cv2.contourArea(c)
        if area < MIN_CONTOUR_AREA:
            continue
        x, y, w, h = cv2.boundingRect(c)
        dist = distance_mm(h)
        ang = horizontal_angle(x + w / 2)
        if best is None or dist < best.distance_mm:
            best = Detection(color=color, rect=(x, y, w, h), distance_mm=dist, angle_rad=ang)
    return best


def decide_mangle(blue: Optional[Detection], red: Optional[Detection]) -> int:
    """Return steering angle (deg) with priority rules."""
    candidate = 0
    if blue and blue.distance_mm < MAX_EFFECTIVE_DISTANCE_MM:
        candidate = np.degrees(blue.angle_rad) + BLUE_ADJUST_DEG
    if red and red.distance_mm < MAX_EFFECTIVE_DISTANCE_MM:
        # If both present choose nearer
        if (not blue) or (red.distance_mm <= blue.distance_mm):
            candidate = np.degrees(red.angle_rad) + RED_ADJUST_DEG
    return int(candidate)


def annotate(frame, det: Detection, color_bgr):
    x, y, w, h = det.rect
    cv2.rectangle(frame, (x, y), (x + w, y + h), color_bgr, 2)
    cv2.putText(frame, f"{det.color}: {det.distance_mm:.1f}mm", (x, y - 25), cv2.FONT_HERSHEY_SIMPLEX, 0.55, color_bgr, 2)
    cv2.putText(frame, f"ang {np.degrees(det.angle_rad):.1f}deg", (x, y - 8), cv2.FONT_HERSHEY_SIMPLEX, 0.5, color_bgr, 1)


def main(show_window: bool = True):
	ser = open_serial(SERIAL_PORT, BAUD_RATE)
	cap = cv2.VideoCapture(0)
	if not cap.isOpened():
		print("[ERROR] Camera not accessible.")
		return

	try:
		while True:
			ok, frame = cap.read()
			if not ok:
				print("[WARN] Frame grab failed")
				break
			hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
			mask_blue = build_mask(hsv, HSV_BLUE_RANGES)
			mask_red = build_mask(hsv, HSV_RED_RANGES)
			contours_blue, _ = cv2.findContours(mask_blue, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
			contours_red, _ = cv2.findContours(mask_red, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
			blue_det = closest_detection(contours_blue, "blue")
			red_det = closest_detection(contours_red, "red")
			mangle = decide_mangle(blue_det, red_det)
			if ser:
				ser.write(f"{mangle}\n".encode())
			if show_window:
				if blue_det:
					annotate(frame, blue_det, (255, 0, 0))
				if red_det:
					annotate(frame, red_det, (0, 0, 255))
				cv2.putText(frame, f"mangle {mangle}", (10, 25), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0,255,0), 2)
				cv2.imshow("Vision", frame)
				if cv2.waitKey(1) & 0xFF == ord('q'):
					break
	except KeyboardInterrupt:
			print("[INFO] Interrupted by user")
	finally:
			cap.release()
			if show_window:
				cv2.destroyAllWindows()
			if ser:
				ser.close()


if __name__ == "__main__":
main()
