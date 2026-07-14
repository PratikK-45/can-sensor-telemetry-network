"""
CAN -> Ethernet gateway, SocketCAN-bypass version.

Reads CAN frames directly off the MCP2515 over SPI (no can0 interface
involved at all) and forwards each frame to your laptop over UDP.

Run on the Raspberry Pi:
    python3 can_udp_gateway.py
"""

import socket
import sys

from mcp2515 import MCP2515
import config


def main():
    print("Initializing MCP2515 (direct SPI, no SocketCAN)...")
    can = MCP2515(
        bus=config.SPI_BUS,
        device=config.SPI_DEVICE,
        speed_hz=config.SPI_SPEED_HZ,
        osc_mhz=config.OSC_MHZ,
        bitrate=config.CAN_BITRATE,
    )

    try:
        can.begin()
    except TimeoutError:
        print("ERROR: MCP2515 did not respond. Check wiring, CS line, "
              "and that no kernel mcp251x driver is holding the SPI bus.")
        sys.exit(1)

    print(f"MCP2515 up at {config.CAN_BITRATE} bps. Listening...")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
        while True:
            frame = can.recv(timeout=None)
            if frame is None:
                continue

            msg = (
                f"ID={hex(frame.arb_id)} "
                f"DLC={frame.dlc} "
                f"DATA={' '.join(f'{b:02X}' for b in frame.data)}"
            )
            sock.sendto(msg.encode(), (config.LAPTOP_IP, config.LAPTOP_PORT))
            print("Sent:", msg)
    except KeyboardInterrupt:
        print("\nStopping.")
    finally:
        sock.close()
        can.close()


if __name__ == "__main__":
    main()
