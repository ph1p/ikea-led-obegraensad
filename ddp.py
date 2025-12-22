#!/usr/bin/env python3

import imageio.v3 as iio
import logging
import numpy as np
import socket
import argparse
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


def create_packet(pixels=None):
    """Create a DDP packet with specified pixels or all off"""
    # Header: 10 bytes
    packet = bytearray(
        [
            0x41,  # Version 1
            0x00,  # Flags
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,  # Reserved
        ]
    )

    # Initialize all pixels to 0
    data = bytearray([0] * (16 * 16 * 3))

    # Set specified pixels
    if pixels:
        for x, y, brightness in pixels:
            if 0 <= x < 16 and 0 <= y < 16 and 0 <= brightness <= 255:
                index = (y * 16 + x) * 3
                data[index : index + 3] = [brightness] * 3

    packet.extend(data)
    return packet


def send_ddp_packet(ip, port, packet):
    """Send a DDP packet to the specified IP and port"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.sendto(packet, (ip, port))
        logger.info(f"Sent DDP packet to {ip}:{port}")
        logger.info(f"Packet size: {len(packet)} bytes")
    except Exception as e:
        logger.error(f"Error: {e}")
    finally:
        sock.close()


def main():
    # Use parent parser for common arguments
    parent_parser = argparse.ArgumentParser(
        description="The parent parser", add_help=False
    )

    parent_parser.add_argument(
        "--ip", default="192.168.178.50", help="IP address of the display"
    )
    parent_parser.add_argument("--port", type=int, default=4048, help="UDP port")
    parent_parser.add_argument(
        "-d", "--debug", action="store_true", help="Enable debug logging"
    )
    parent_parser.add_argument(
        "-v", "--verbose", action="store_true", help="Enable verbose logging"
    )

    # Main parser with subcommands
    parser = argparse.ArgumentParser(
        description="Send DDP packets to control LED matrix", parents=[parent_parser]
    )

    subparsers = parser.add_subparsers(help="help for subcommand", dest="subcommand")

    subparsers.add_parser("clear", help="Clear all pixels", parents=[parent_parser])

    fill_parser = subparsers.add_parser(
        "fill",
        help="Fill all pixels with specified brightness",
        parents=[parent_parser],
    )
    fill_parser.add_argument(
        "brightness",
        type=int,
        metavar="BRIGHTNESS",
        choices=range(0, 256),
        help="Brightness level (0-255)",
    )

    pixels_parser = subparsers.add_parser(
        "pixels", help="Set individual pixel brightness", parents=[parent_parser]
    )
    pixels_parser.add_argument(
        "-p",
        "--pixel",
        nargs=3,
        type=int,
        action="append",
        metavar=("X", "Y", "BRIGHTNESS"),
        help="Set pixel at X,Y to brightness (can be used multiple times)",
    )

    video_parser = subparsers.add_parser(
        "video", help="Play video on LED matrix via DDP", parents=[parent_parser]
    )
    video_parser.add_argument(
        "video_file", type=str, metavar="VIDEO_FILE", help="Path to input video file"
    )

    # Deprecated arguments for backward compatibility
    parser.add_argument(
        "--clear", action="store_true", help="Clear all pixels", deprecated=True
    )
    parser.add_argument(
        "--fill",
        type=int,
        metavar="BRIGHTNESS",
        help="Fill all pixels with specified brightness (0-255)",
        deprecated=True,
    )
    parser.add_argument(
        "-p",
        "--pixel",
        nargs=3,
        type=int,
        action="append",
        metavar=("X", "Y", "BRIGHTNESS"),
        help="Set pixel at X,Y to brightness (can be used multiple times)",
        deprecated=True,
    )

    args = parser.parse_args()

    logging.basicConfig(
        level=logging.DEBUG
        if args.debug
        else logging.INFO
        if args.verbose
        else logging.WARNING
    )

    pixels = []

    # Validate fill brightness
    if args.fill is not None:
        if not 0 <= args.fill <= 255:
            parser.error("Fill brightness must be between 0 and 255")
        pixels = [(x, y, args.fill) for x in range(16) for y in range(16)]

    if args.subcommand == "fill":
        if not 0 <= args.brightness <= 255:
            parser.error("Fill brightness must be between 0 and 255")
            return

        logger.info(f"Filling all pixels with brightness {args.brightness}")
        pixels = [(x, y, args.brightness) for x in range(16) for y in range(16)]

    # Validate pixel coordinates and brightness
    if args.subcommand == "pixels" or args.pixel is not None:
        for x, y, brightness in args.pixel:
            if not (0 <= x < 16 and 0 <= y < 16):
                parser.error(f"Invalid coordinates: {x},{y} (must be 0-15)")
            if not (0 <= brightness <= 255):
                parser.error(f"Invalid brightness: {brightness} (must be 0-255)")

            logger.info(f"Setting pixel ({x},{y}) to brightness {brightness}")
            pixels.append((x, y, brightness))

    if args.subcommand == "clear" or args.clear:
        logger.info("Clearing all pixels")

    if args.subcommand == "video":
        play_video(args.video_file, args.ip, args.port)
    else:
        packet = create_packet(pixels)
        send_ddp_packet(args.ip, args.port, packet)

    return


if __name__ == "__main__":
    main()
