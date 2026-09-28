# goal_detection - By: yuets - Sun Apr 5 2026

import sensor, image, time
from machine import UART
import struct
from fpioa_manager import fm
from Maix import GPIO
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)
from fpioa_manager import *
fm.register(34, fm.fpioa.UART1_TX)
fm.register(35,fm.fpioa.UART1_RX)
uart_out = UART(UART.UART1, 115200, 8, None, 1, timeout=1000, read_buf_len=4096)

fm.register(18, fm.fpioa.GPIO1)
ButtonA=GPIO(GPIO.GPIO1, GPIO.IN, GPIO.PULL_UP)
fm.register(19, fm.fpioa.GPIO2)
ButtonB=GPIO(GPIO.GPIO2, GPIO.IN, GPIO.PULL_UP)

clock = time.clock()
detect_yellow = 1

last_pressed = 0

thresholds = [[0, 80, 40, 80, 10, 80]]      # red
# thresholds = [[0, 80, -120, -10, 0, 30]]    # green
# thresholds = [[0, 80, 30, 100, -120, -60]]  # blue

yellow_threshold = [[50, 90, 0, 30, 20, 40]]
blue_threshold = [[20, 50, 20, 40, -100, 0]]
# calibrate these when you need to
while 1:
    clock.tick()

    current_pressed = not ButtonA.value()

    if (current_pressed and last_pressed == False):
        detect_yellow = int(not detect_yellow)
    print(detect_yellow)

    img = sensor.snapshot()
    img.rotation_corr(z_rotation=-90)
    blobs = []
    if detect_yellow:
        blobs = img.find_blobs(yellow_threshold, pixels_threshold=100)
    if not detect_yellow:
        blobs = img.find_blobs(blue_threshold, pixels_threshold=100)
    center = [-1, -1]
    if blobs:
        largest_blob = max(blobs, key=lambda b: b.pixels())
        center = (int(largest_blob[0] + largest_blob[2] / 2),
                      int(largest_blob[1] + largest_blob[3] / 2))
        img.draw_circle(center[0], center[1], 5, (255, 0, 0), fill=True)

    #print(center[0], center[1])
    #print("fps: ", clock.fps())
    packet = struct.pack('<Bii', ord('e'), center[0], center[1])  # 1-byte header + 2x 4-byte ints
    uart_out.write(packet)
    #print("wrote something")
    last_pressed = current_pressed
