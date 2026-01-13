#!/usr/bin/env python3

import logging
import socket
import argparse

logger: logging.Logger = logging.getLogger(__name__)


def create_packet(pixels: list[tuple[int, int, int]] | None = None) -> bytearray:
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
                index: int = (y * 16 + x) * 3
                data[index : index + 3] = [brightness] * 3

    packet.extend(data)
    return packet


def send_ddp_packet(ip: str, port: int, packet: bytearray) -> None:
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


def create_arg_parser() -> argparse.ArgumentParser:
    # Use parent parser for common arguments
    parent_parser = argparse.ArgumentParser(
        description="The parent parser", add_help=False
    )

    parent_parser.add_argument(
        "--ip", type=str, default="192.168.178.50", help="IP address of the display"
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

    fill_parser: argparse.ArgumentParser = subparsers.add_parser(
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

    pixels_parser: argparse.ArgumentParser = subparsers.add_parser(
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

    return parser


def main() -> None:
    parser: argparse.ArgumentParser = create_arg_parser()
    args: argparse.Namespace = parser.parse_args()

    logging.basicConfig(
        level=logging.DEBUG
        if args.debug
        else logging.INFO
        if args.verbose
        else logging.WARNING
    )

    pixels: list[tuple[int, int, int]] = []

    # Validate fill brightness
    if args.subcommand == "fill" or args.fill is not None:
        if args.subcommand == "fill":
            fill_brightness: int = args.brightness
        else:
            fill_brightness: int = args.fill

        if not 0 <= fill_brightness <= 255:
            parser.error("Fill brightness must be between 0 and 255")

        logger.info(f"Filling all pixels with brightness {fill_brightness}")
        pixels = [(x, y, fill_brightness) for x in range(16) for y in range(16)]

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

    packet: bytearray = create_packet(pixels)
    send_ddp_packet(args.ip, args.port, packet)

    return


if __name__ == "__main__":
    main()
