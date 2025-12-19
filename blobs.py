#!/usr/bin/env python3
import socket
import numpy as np
import time

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

IP = "192.168.178.50"
PORT = 4048
FPS = 40

# ==============================
# DDP sender
# ==============================
def send_ddp_packet(sock, ip, port, packet):
    sock.sendto(packet, (ip, port))

# ==============================
# Metaball simulation
# ==============================
balls = np.random.uniform(0, [SIM_W, SIM_H], size=(NUM_BALLS, 2))
velocities = np.random.uniform(-SPEED, SPEED, size=(NUM_BALLS, 2))

# Pre-compute coordinate grids for vectorized rendering
y_coords, x_coords = np.mgrid[0:SIM_H, 0:SIM_W]
coords = np.stack([x_coords, y_coords], axis=-1).astype(float)

def attenuation_fn(d_squared, radius=RADIUS):
    """Vectorized attenuation using squared distance"""
    mask = d_squared <= radius * radius
    ratio = d_squared / (radius * radius)
    result = np.where(mask, (1 - ratio)**2, 0.0)
    return result

def tone_map(grid, gamma=GAMMA):
    """Vectorized tone mapping"""
    capped = np.minimum(grid, CAP_VALUE)
    normalized = capped / CAP_VALUE
    return (normalized ** (1 / gamma) * 255).astype(np.uint8)

def update_positions():
    global balls, velocities
    balls += velocities

    # Bounce off walls
    out_of_bounds_x = (balls[:, 0] < 0) | (balls[:, 0] >= SIM_W)
    out_of_bounds_y = (balls[:, 1] < 0) | (balls[:, 1] >= SIM_H)
    velocities[out_of_bounds_x, 0] *= -1
    velocities[out_of_bounds_y, 1] *= -1

    # Clamp positions
    balls[:, 0] = np.clip(balls[:, 0], 0, SIM_W - 1)
    balls[:, 1] = np.clip(balls[:, 1], 0, SIM_H - 1)

def render_highres():
    """Vectorized metaball rendering - ~100x faster"""
    grid = np.zeros((SIM_H, SIM_W), dtype=float)

    for ball_pos in balls:
        # Calculate squared distances to all pixels (avoids sqrt)
        diff = coords - ball_pos
        d_squared = np.sum(diff * diff, axis=-1)
        grid += attenuation_fn(d_squared)

    return grid

def downsample_and_create_packet(grid):
    """Combined downsampling and packet creation - avoids intermediate list"""
    # Tone map the entire grid at once
    tone_mapped = tone_map(grid)

    # Downsample using reshape and mean
    reshaped = tone_mapped.reshape(HEIGHT, SIM_SCALE, WIDTH, SIM_SCALE)
    downsampled = reshaped.mean(axis=(1, 3)).astype(np.uint8)

    # Create packet header
    packet = bytearray([0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])

    # Flatten and interleave RGB (all same value for grayscale)
    flat = downsampled.flatten()
    data = np.repeat(flat, 3)
    packet.extend(data.tobytes())

    return packet

# ==============================
# Main loop
# ==============================
def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    frame_time = 1.0 / FPS

    try:
        while True:
            start_time = time.time()

            update_positions()
            highres = render_highres()
            packet = downsample_and_create_packet(highres)
            send_ddp_packet(sock, IP, PORT, packet)

            # Compensate for processing time
            elapsed = time.time() - start_time
            sleep_time = max(0, frame_time - elapsed)
            time.sleep(sleep_time)

    except KeyboardInterrupt:
        pass
    finally:
        sock.close()

if __name__ == "__main__":
    main()
