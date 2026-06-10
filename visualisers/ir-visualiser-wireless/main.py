# run this file
# change or add test sets (such as test_set_1) and follow same format
# raw analog ir values in order of TSSP 1 to 16
# white lines are the raw sensor analog readings
# the red line shows the calculated angle and "strength"

# connect to the ir sensor board by connecting to the wifi it creates
# it must have the newest firmware. 
# ssid: LEGACY_IR
# password: legacy_robot

# only 1 device should be able to connect to the board.
import pygame as pg
from ir_calc import *
import serial
import struct
import math
import struct
import socket


timeout = 0.01  # small timeout for non-blocking reads


FLOAT_COUNT = 16


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

# ===== SOCKET SETUP =====
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setblocking(False)
sock.bind(("0.0.0.0", 1234))


# ===== MAIN LOOP =====
while running:
    events = pg.event.get()
    for event in events:
        if event.type == pg.QUIT:
            running = False

    # ===== READING FROM THE IR =====
    try:
        data, addr = sock.recvfrom(1024)
        if len(data) == 64:
            values = struct.unpack("16f", data)
            ir_calc.update(values)
    except BlockingIOError:
        print("havent received yet")

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


pg.quit()