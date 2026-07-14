# --- SPI / MCP2515 ---
SPI_BUS = 0
SPI_DEVICE = 0        # 0 = CE0 (GPIO8), 1 = CE1 (GPIO7)
SPI_SPEED_HZ = 1_000_000
OSC_MHZ = 8
CAN_BITRATE = 500000  # must match the STM32's bxCAN configuration

# --- UDP ---
LAPTOP_IP = "192.168.1.100"  # <-- change to your laptop's actual IP
LAPTOP_PORT = 5000
