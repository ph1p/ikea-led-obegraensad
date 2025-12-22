#!/usr/bin/env python3

import logging
import socket
import argparse

logger = logging.getLogger(__name__)


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


def send_ddp_packet(ip, port, packet, debug=False):
    """Send a DDP packet to the specified IP and port"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.sendto(packet, (ip, port))
        if debug:
            print(f"Sent DDP packet to {ip}:{port}")
            print(f"Packet size: {len(packet)} bytes")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        sock.close()


def main():
    parser = argparse.ArgumentParser(
        description="Send DDP packets to control LED matrix"
    )
    parser.add_argument(
        "--ip", default="192.168.178.50", help="IP address of the display"
    )
    parser.add_argument("--port", type=int, default=4048, help="UDP port")
    parser.add_argument(
        "-d", "--debug", action="store_true", help="Enable debug logging"
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Enable verbose logging"
    )

    subparsers = parser.add_subparsers(help="help for subcommand", dest="subcommand")

    subparsers.add_parser("clear", help="Clear all pixels")

    fill_parser = subparsers.add_parser(
        "fill", help="Fill all pixels with specified brightness"
    )
    fill_parser.add_argument(
        "brightness",
        type=int,
        metavar="BRIGHTNESS",
        choices=range(0, 256),
        help="Brightness level (0-255)",
    )

    pixels_parser = subparsers.add_parser(
        "pixels", help="Set individual pixel brightness"
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
        "video", help="Play video on LED matrix via DDP"
    )
    video_parser.add_argument("video_file", type=str, help="Path to input video file")

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

    # Validate fill brightness
    if args.subcommand == "fill":
        pixels = []
        if not 0 <= args.brightness <= 255:
            parser.error("Fill brightness must be between 0 and 255")
            return

        pixels = [(x, y, args.brightness) for x in range(16) for y in range(16)]

    # Validate pixel coordinates and brightness
    if args.subcommand == "pixel":
        if not (0 <= args.x < 16 and 0 <= args.y < 16):
            parser.error(f"Invalid coordinates: {args.x},{args.y} (must be 0-15)")

        if not (0 <= args.brightness <= 255):
            parser.error(f"Invalid brightness: {args.brightness} (must be 0-255)")

        pixels = [(args.x, args.y, args.brightness)]

        for x, y, brightness in args.pixel:
            if not (0 <= x < 16 and 0 <= y < 16):
                parser.error(f"Invalid coordinates: {x},{y} (must be 0-15)")
            if not (0 <= brightness <= 255):
                parser.error(f"Invalid brightness: {brightness} (must be 0-255)")
            pixels.append((x, y, brightness))

    if args.subcommand == "clear":
        pixels = []

    if args.subcommand in ["clear", "fill", "pixel"]:
        packet = create_packet(pixels)
        send_ddp_packet(args.ip, args.port, packet)

    if args.subcommand == "video":
        from videoplayer import play_video

        play_video(args.video_file, args.ip, args.port, args.debug, args.verbose)
        return


if __name__ == "__main__":
    main()
