from ddp import create_packet, send_ddp_packet

import os
import imageio.v3 as iio
import numpy as np
import time

path = "C:\\Users\\Thomas\\Dev\\bad-apple"
file = "badapple_480_16x16.mp4"

im = iio.imread(os.path.join(path, file), plugin="pyav")

# manually convert the video to grayscale
gray_frames = np.dot(im[..., :3], [0.2989, 0.5870, 0.1140])

# Get video metadata for proper playback timing
video_meta = iio.immeta(os.path.join(path, file), plugin="pyav")
fps = video_meta.get("fps", 30)  # Default to 30 if not found
frame_delay = 1.0 / fps

print(f"Playing at {fps} FPS")

# Track timing
start_time = time.perf_counter()

# Iterate over frames with enumerate
for frame_count, frame in enumerate(gray_frames):
    # Convert frame to pixels list
    pixels = []
    for y in range(16):
        for x in range(16):
            brightness = int(frame[y, x])
            pixels.append((x, y, brightness))

    # Create and send the DDP packet
    packet = create_packet(pixels)
    send_ddp_packet("192.168.1.51", 4048, packet)

    # Calculate when the next frame should be displayed
    target_time = start_time + ((frame_count + 1) * frame_delay)
    current_time = time.perf_counter()

    # Sleep only if we're ahead of schedule
    sleep_time = target_time - current_time
    if sleep_time > 0:
        time.sleep(sleep_time)
    elif sleep_time < -frame_delay:
        # Warn if we're falling behind by more than one frame
        print(f"Warning: Frame {frame_count} is {-sleep_time:.3f}s behind")

print(f"Playback complete. Total time: {time.perf_counter() - start_time:.2f}s")
