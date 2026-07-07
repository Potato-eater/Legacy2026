import serial
import struct
import pygame
from pygame.locals import *
import sys
import math
import time  # Imported for timing updates
width, height = 640, 480
# --- CONFIGURATION ---
SERIAL_PORT = 'COM8' 
BAUD_RATE = 115200
BOT_TIMEOUT = 1.0  # Seconds before a bot disappears if no data is received

class Vector():
    def __init__(self, i, j):
        self.i = i
        self.j = j

class BotData():
    def __init__(self, unpacked_data):
        self.heading = unpacked_data[1]
        self.pos_vector = Vector(unpacked_data[2], unpacked_data[3])
        self.ball_strength = unpacked_data[4]
        self.ball_angle = unpacked_data[5]
        self.line_vector = Vector(unpacked_data[6], unpacked_data[7])
        self.velocity = Vector(unpacked_data[8], unpacked_data[9])
        self.cam_x = unpacked_data[10]
        self.last_seen = time.time()  # Track when this specific data was unpacked
    

    def draw(self, screen):
        center_x = int(width // 2 + self.pos_vector.i * 5)
        center_y = int(height // 2 - self.pos_vector.j * 5)
        pygame.draw.circle(screen, (255, 0, 0), (center_x, center_y), 20)
        pygame.draw.line(screen, (0, 255, 0), (center_x, center_y), (center_x + 20 * math.cos(self.heading + math.pi/2), center_y - 20 * math.sin(self.heading + math.pi/2)), 5)


# --- STRUCT FORMAT CONFIGURATION ---
STRUCT_FORMAT = '<3s f ffff ffff i B 3s'
EXPECTED_SIZE = struct.calcsize(STRUCT_FORMAT)

def main():
    pygame.init()
    pygame.font.init()
    my_font = pygame.font.SysFont('Comic Sans MS', 24)
    fps = 60
    fpsClock = pygame.time.Clock()
    

    screen = pygame.display.set_mode((width, height))

    print(f"Connecting to {SERIAL_PORT}...")
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0) # Non-blocking
    except Exception as e:
        print(f"Error opening serial port: {e}")
        return

    print("Listening with stream-aligned validation...")
    
    data_buffer = bytearray()
    botdatas = {}

    while True:
        # --- 1. ACCUMULATE STREAM DATA ---
        bytes_waiting = ser.in_waiting
        if bytes_waiting > 0:
            data_buffer.extend(ser.read(bytes_waiting))

        # --- 2. PARSE ALL COMPLETED FRAMES IN THE BUFFER ---
        while len(data_buffer) >= EXPECTED_SIZE:
            header_idx = data_buffer.find(b'eee')
            
            if header_idx > 0:
                del data_buffer[:header_idx]
                continue
                
            if header_idx == -1:
            # Prevent buffer runaway if no header exists by keeping only the last 2 bytes
                if len(data_buffer) > 2:
                    del data_buffer[:-2]
                break

            footer_idx = EXPECTED_SIZE - 3
            if data_buffer[footer_idx:footer_idx+3] == b'fff':
                packet = data_buffer[:EXPECTED_SIZE]
                
                try:
                    unpacked_data = struct.unpack(STRUCT_FORMAT, packet)
                    bot_id = unpacked_data[11]
                    # Update or add the bot class object
                    botdatas[bot_id] = BotData(unpacked_data)
                    
                except struct.error:
                    pass
                
                del data_buffer[:EXPECTED_SIZE]
            else:
                del data_buffer[0:1]

        # --- 3. PYGAME EVENT HANDLING ---
        for event in pygame.event.get():
            if event.type == QUIT:
                ser.close()
                pygame.quit()
                sys.exit()
        
        # --- 4. TIMEOUT FILTERING ---
        current_time = time.time()
        # Clean out dead records so old data doesn't stick around on-screen
        botdatas = {id: bot for id, bot in botdatas.items() if current_time - bot.last_seen < BOT_TIMEOUT}

        # --- 5. RENDERING ---
        screen.fill((0, 0, 0)) 
        
        for bot_id, bot in botdatas.items():
            if bot_id == 0:
                continue
                
            center_x = int(width // 2 + bot.pos_vector.i * 5)
            center_y = int(height // 2 - bot.pos_vector.j * 5)
            
            # Draw tracking asset elements
            bot.draw(screen)
            text_surface = my_font.render(f'Bot {bot_id}', True, (255, 255, 255))
            screen.blit(text_surface, (center_x + 12, center_y - 12))
            
        pygame.display.flip()
        fpsClock.tick(fps)

if __name__ == "__main__":
    main()