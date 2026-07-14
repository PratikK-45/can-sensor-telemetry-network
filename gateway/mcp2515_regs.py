"""
MCP2515 register map and SPI instruction set.
Reference: Microchip MCP2515 datasheet (DS20001801).
"""

# ---- SPI Instructions ----
INSTR_RESET = 0xC0
INSTR_READ = 0x03
INSTR_WRITE = 0x02
INSTR_RTS = 0x80          # OR with 0x01/0x02/0x04 for TXB0/1/2
INSTR_READ_STATUS = 0xA0
INSTR_RX_STATUS = 0xB0
INSTR_BIT_MODIFY = 0x05
INSTR_READ_RXB0 = 0x90    # read RX buffer 0, starting at SIDH
INSTR_READ_RXB1 = 0x94    # read RX buffer 1, starting at SIDH
INSTR_LOAD_TXB0 = 0x40    # load TX buffer 0, starting at SIDH

# ---- Control / Status Registers ----
CANSTAT = 0x0E
CANCTRL = 0x0F
CNF3 = 0x28
CNF2 = 0x29
CNF1 = 0x2A
CANINTE = 0x2B
CANINTF = 0x2C
EFLG = 0x2D
TEC = 0x1C
REC = 0x1D

# ---- RX Buffer 0 ----
RXB0CTRL = 0x60
RXB0SIDH = 0x61
RXB0SIDL = 0x62
RXB0EID8 = 0x63
RXB0EID0 = 0x64
RXB0DLC = 0x65
RXB0D0 = 0x66

# ---- RX Buffer 1 ----
RXB1CTRL = 0x70
RXB1SIDH = 0x71
RXB1SIDL = 0x72
RXB1EID8 = 0x73
RXB1EID0 = 0x74
RXB1DLC = 0x75
RXB1D0 = 0x76

# ---- TX Buffer 0 ----
TXB0CTRL = 0x30
TXB0SIDH = 0x31
TXB0SIDL = 0x32
TXB0EID8 = 0x33
TXB0EID0 = 0x34
TXB0DLC = 0x35
TXB0D0 = 0x36

# ---- Masks / Filters (only need RXM0 for "accept everything") ----
RXM0SIDH = 0x20
RXM0SIDL = 0x21
RXM1SIDH = 0x24
RXM1SIDL = 0x25

# ---- CANCTRL: REQOP bits [7:5] ----
MODE_NORMAL = 0x00
MODE_SLEEP = 0x20
MODE_LOOPBACK = 0x40
MODE_LISTENONLY = 0x60
MODE_CONFIG = 0x80
MODE_MASK = 0xE0

# ---- CANSTAT: OPMOD bits [7:5], read-only mirror of current mode ----
OPMOD_MASK = 0xE0

# ---- CANINTF flag bits ----
RX0IF = 0x01
RX1IF = 0x02
TX0IF = 0x04
TX1IF = 0x08
TX2IF = 0x10
ERRIF = 0x20
WAKIF = 0x40
MERRF = 0x80

# ---- RXB0CTRL ----
# RXM<1:0> = 11 -> receive any message, filters/masks disabled
RXB0CTRL_RXM_ANY = 0x60
RXB0CTRL_BUKT = 0x04   # rollover to RXB1 if RXB0 full

# ---- RXB1CTRL ----
RXB1CTRL_RXM_ANY = 0x60

# ---- Bit timing configuration tables ----
# (CNF1, CNF2, CNF3) for an 8 MHz crystal, common bitrates.
# Values taken from the standard MCP2515 timing tables used across
# widely-deployed open-source drivers (e.g. autowp/mcp2515, Seeed-Studio).
BITRATE_8MHZ = {
    125000:  (0x01, 0xB1, 0x85),
    250000:  (0x00, 0xB1, 0x85),
    500000:  (0x00, 0x90, 0x02),
    1000000: (0x00, 0x80, 0x00),
}
