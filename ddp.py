#!/usr/bin/env python3
import socket
import argparse

def create_packet(pixels=None):
    """Create a DDP packet with specified pixels or all off"""
    # Header: 10 bytes
    packet = bytearray([
        0x41,  # Version 1
        0x00,  # Flags
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  # Reserved
    ])

    # Initialize all pixels to 0
    data = bytearray([0] * (16 * 16 * 3))

    # Set specified pixels
    if pixels:
        for x, y, brightness in pixels:
            if 0 <= x < 16 and 0 <= y < 16 and 0 <= brightness <= 255:
                index = (y * 16 + x) * 3
                data[index:index + 3] = [brightness] * 3

    packet.extend(data)
    return packet

def send_ddp_packet(ip, port, packet):
    """Send a DDP packet to the specified IP and port"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.sendto(packet, (ip, port))
        print(f"Sent DDP packet to {ip}:{port}")
        print(f"Packet size: {len(packet)} bytes")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        sock.close()

def main():
    parser = argparse.ArgumentParser(description='Send DDP packets to control LED matrix')
    parser.add_argument('--ip', default='192.168.178.50', help='IP address of the display')
    parser.add_argument('--port', type=int, default=4048, help='UDP port')
    parser.add_argument('--clear', action='store_true', help='Clear all pixels')
    parser.add_argument('--fill', type=int, metavar='BRIGHTNESS',
                       help='Fill all pixels with specified brightness (0-255)')
    parser.add_argument('--pixel', nargs=3, type=int, action='append',
                       metavar=('X', 'Y', 'BRIGHTNESS'),
                       help='Set pixel at X,Y to brightness (can be used multiple times)')

    args = parser.parse_args()

    # Validate fill brightness
    if args.fill is not None and not 0 <= args.fill <= 255:
        parser.error("Fill brightness must be between 0 and 255")

    # Validate pixel coordinates and brightness
    pixels = []
    if args.pixel:
        for x, y, brightness in args.pixel:
            if not (0 <= x < 16 and 0 <= y < 16):
                parser.error(f"Invalid coordinates: {x},{y} (must be 0-15)")
            if not (0 <= brightness <= 255):
                parser.error(f"Invalid brightness: {brightness} (must be 0-255)")
            pixels.append((x, y, brightness))

    # Create appropriate packet
    if args.fill is not None:
        pixels = [(x, y, args.fill) for x in range(16) for y in range(16)]

    packet = create_packet(pixels)
    send_ddp_packet(args.ip, args.port, packet)

if __name__ == "__main__":
    main()