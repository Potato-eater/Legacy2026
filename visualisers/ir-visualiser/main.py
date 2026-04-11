# run this file
# change or add test sets (such as test_set_1) and follow same format
# raw analog ir values in order of TSSP 1 to 16
# white lines are the raw sensor analog readings
# the red line shows the calculated angle and "strength"

# newer program, with uart debugging because i messed up the usb
import pygame as pg
from ir_calc import *
import serial
import struct
import math

port = 'COM18'
baud_rate = 921600
timeout = 0.01  # small timeout for non-blocking reads

# ===== UART PACKET CONFIG =====
HEADER = b'e'
FLOAT_COUNT = 20
PACKET_SIZE = FLOAT_COUNT * 4  # 80 bytes

pg.init()

# ===== TEST DATA =====
test_set_5 = [
    100, 100, 100, 100, 100, 10, 10, 20,
    10, 5, 2, 3, 5, 2, 100, 100,
]

ir_calc = IRCalculator()

# Initial test
ir_calc.update(test_set_5)
print("angle", ir_calc.angle)
print("angle degrees", ir_calc.angle * 180 / math.pi)
print("magnitude", ir_calc.magnitude)

# ===== DISPLAY SETUP =====
running = True
screen_info = pg.display.Info()
DIM = screen_info.current_h
SCALE = 1
screen = pg.display.set_mode((DIM, DIM))
clock = pg.time.Clock()

# ===== SERIAL SETUP =====
buffer = b""

try:
    ser = serial.Serial(port, baud_rate, timeout=timeout)
    print(f"Listening on {port}...")
except serial.SerialException as e:
    print(f"Could not open serial port {port}: {e}")
    ser = None

# ===== MAIN LOOP =====
while running:
    events = pg.event.get()
    for event in events:
        if event.type == pg.QUIT:
            running = False

    # ===== READ SERIAL (BINARY MODE) =====
    if ser:
        buffer += ser.read(128)

        while True:
            idx = buffer.find(HEADER)

            if idx == -1:
                buffer = b""  # discard garbage
                break

            if len(buffer) < idx + 1 + PACKET_SIZE:
                break  # wait for full packet

            # Extract packet (skip header)
            packet = buffer[idx + 1 : idx + 1 + PACKET_SIZE]

            # Remove processed data
            buffer = buffer[idx + 1 + PACKET_SIZE :]

            try:
                floats = struct.unpack('<20f', packet)

                # Ignore first 4 floats, take next 16
                values = list(floats[4:20])

                print("Received:", values)
                ir_calc.update(values)

                # Optional debug check
                if any(abs(v) > 10000 for v in values):
                    print("⚠️ suspicious values:", values)

            except struct.error:
                print("Ignored: unpacking error")

    # ===== DEBUG OUTPUT =====
    print("angle", ir_calc.angle)
    print("angle degrees", ir_calc.angle * 180 / math.pi)
    print("magnitude", ir_calc.magnitude)

    # ===== DRAW =====
    screen.fill("BLACK")

    # Draw IR vectors
    lis = ir_calc.ir_vectors[0:15]
    for ir_vector in lis:
        pg.draw.line(
            screen,
            "WHITE",
            (DIM / 2, DIM / 2),
            (ir_vector.i * SCALE + DIM / 2, -ir_vector.j * SCALE + DIM / 2),
            2
        )

    # Draw resultant vector
    pg.draw.line(
        screen,
        "RED",
        (DIM / 2, DIM / 2),
        (ir_calc.ir_vec.i * SCALE + DIM / 2, -ir_calc.ir_vec.j * SCALE + DIM / 2),
        3
    )

    pg.display.flip()
    clock.tick(60)

# ===== CLEANUP =====
if ser:
    ser.close()

pg.quit()