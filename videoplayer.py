import argparse
from ddp import create_packet

import imageio.v3 as iio
import logging
import numpy as np
import socket
import time

logger = logging.getLogger(__name__)


def play_video(video_file_path, ip, port=4048):
    # Load video frames using imageio with pyav plugin
    im = iio.imread(video_file_path, plugin="pyav")

    # manually convert the video to grayscale
    gray_frames = np.dot(im[..., :3], [0.2989, 0.5870, 0.1140])

    # Get video metadata for proper playback timing
    video_meta = iio.immeta(video_file_path, plugin="pyav")
    fps = video_meta.get("fps", 30)  # Default to 30 if not found
    frame_delay = 1.0 / fps

    logger.info(f"Playing at {fps} FPS")

    # Track timing
    start_time = time.perf_counter()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
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
            sock.sendto(packet, (ip, port))

            # Calculate when the next frame should be displayed
            target_time = start_time + ((frame_count + 1) * frame_delay)
            current_time = time.perf_counter()

            # Sleep only if we're ahead of schedule
            sleep_time = target_time - current_time
            if sleep_time > 0:
                time.sleep(sleep_time)
            elif sleep_time < -frame_delay:
                # Warn if we're falling behind by more than one frame
                logger.warning(
                    f"Warning: Frame {frame_count} is {-sleep_time:.3f}s behind"
                )

        logger.info(
            f"Playback complete. Total time: {time.perf_counter() - start_time:.2f}s"
        )
    except KeyboardInterrupt:
        logger.warning("Playback interrupted by user.")
    finally:
        sock.close()


def main():
    parser = argparse.ArgumentParser(description="Play video on LED matrix via DDP")

    parser.add_argument(
        "--ip", default="192.168.178.50", help="IP address of the display"
    )
    parser.add_argument("--port", type=int, default=4048, help="UDP port")
    parser.add_argument("-i", "--input", required=True, help="Path to input video file")
    parser.add_argument(
        "-d", "--debug", action="store_true", help="Enable debug logging"
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Enable verbose logging"
    )

    args = parser.parse_args()

    logging.basicConfig(
        level=logging.DEBUG
        if args.debug
        else logging.INFO
        if args.verbose
        else logging.WARNING
    )

    video_file_path = args.input
    ip = args.ip
    port = args.port

    play_video(video_file_path, ip, port)


if __name__ == "__main__":
    main()
