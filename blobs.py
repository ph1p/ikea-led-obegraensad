#!/usr/bin/env python3
import socket
import numpy as np
import time
import math

# ==============================
# Config
# ==============================
WIDTH, HEIGHT = 16, 16       # Display size
SIM_SCALE = 8                # Simulation resolution multiplier
SIM_W, SIM_H = WIDTH*SIM_SCALE, HEIGHT*SIM_SCALE

NUM_BALLS = 4
RADIUS = 3.0 * SIM_SCALE                 # Radius in simulation pixels
SPEED = 0.6 * SIM_SCALE                 # Speed in simulation pixels/frame
GAMMA = 1.7
CAP_VALUE = 3.0              # Max field value before tone-mapping

IP = "192.168.1.112"
PORT = 4048
FPS = 40

# ==============================
# DDP sender
# ==============================
def create_packet(pixels=None):
    packet = bytearray([
        0x41,  # Version 1
        0x00,  # Flags
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    ])
    data = bytearray([0] * (WIDTH * HEIGHT * 3))
    if pixels:
        for x, y, brightness in pixels:
            if 0 <= x < WIDTH and 0 <= y < HEIGHT and 0 <= brightness <= 255:
                idx = (y * WIDTH + x) * 3
                data[idx:idx + 3] = [brightness] * 3
    packet.extend(data)
    return packet

def send_ddp_packet(ip, port, packet):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.sendto(packet, (ip, port))
    finally:
        sock.close()

# ==============================
# Metaball simulation
# ==============================
balls = []
for _ in range(NUM_BALLS):
    x = np.random.uniform(0, SIM_W)
    y = np.random.uniform(0, SIM_H)
    vx = np.random.uniform(-SPEED, SPEED)
    vy = np.random.uniform(-SPEED, SPEED)
    balls.append([x, y, vx, vy])

def attenuation_fn(d, radius=RADIUS):
    if d > radius:
        return 0.0
    return (1 - (d / radius)**2)**2

def tone_map(v, gamma=GAMMA):
    v_capped = min(v, CAP_VALUE)
    n = v_capped / CAP_VALUE
    return int(pow(n, 1 / gamma) * 255)

def update_positions():
    for b in balls:
        b[0] += b[2]
        b[1] += b[3]
        if b[0] < 0 or b[0] >= SIM_W:
            b[2] *= -1
        if b[1] < 0 or b[1] >= SIM_H:
            b[3] *= -1

def render_highres():
    grid = np.zeros((SIM_H, SIM_W), dtype=float)
    for y in range(SIM_H):
        for x in range(SIM_W):
            val = 0.0
            for bx, by, _, _ in balls:
                d = math.dist((x, y), (bx, by))
                val += attenuation_fn(d)
            grid[y, x] = val
    return grid

def downsample(grid):
    """Average SIM_SCALE×SIM_SCALE blocks into one pixel"""
    pixels = []
    for y in range(HEIGHT):
        for x in range(WIDTH):
            block = grid[
                y*SIM_SCALE:(y+1)*SIM_SCALE,
                x*SIM_SCALE:(x+1)*SIM_SCALE
            ]
            avg_val = block.mean()
            brightness = tone_map(avg_val)
            pixels.append((x, y, brightness))
    return pixels

# ==============================
# Main loop
# ==============================
def main():
    try:
        while True:
            update_positions()
            highres = render_highres()
            pixels = downsample(highres)
            packet = create_packet(pixels)
            send_ddp_packet(IP, PORT, packet)
            time.sleep(1 / FPS)
    except KeyboardInterrupt:
        pass

if __name__ == "__main__":
    main()
