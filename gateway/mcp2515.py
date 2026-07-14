"""
Direct-SPI MCP2515 driver for Raspberry Pi (bypasses SocketCAN entirely).

Requires: pip3 install spidev

Wiring reminder (Pi 5, SPI0):
  MCP2515 VCC -> 5V (module has onboard regulator to 3.3V logic)
  MCP2515 GND -> GND
  MCP2515 SCK -> GPIO11 (SPI0 SCLK)
  MCP2515 SI  -> GPIO10 (SPI0 MOSI)
  MCP2515 SO  -> GPIO9  (SPI0 MISO)
  MCP2515 CS  -> GPIO8  (SPI0 CE0) [or GPIO7/CE1, set bus/device below]
  MCP2515 INT -> any free GPIO (optional, for interrupt-driven RX)
"""

import time
import spidev

from mcp2515_regs import *


class CANFrame:
    """Simple container for a received/sent CAN frame."""

    def __init__(self, arb_id, data, dlc=None, extended=False):
        self.arb_id = arb_id
        self.data = bytes(data)
        self.dlc = dlc if dlc is not None else len(data)
        self.extended = extended

    def __repr__(self):
        hexdata = " ".join(f"{b:02X}" for b in self.data)
        idfmt = f"{self.arb_id:08X}" if self.extended else f"{self.arb_id:03X}"
        return f"CANFrame(ID=0x{idfmt} DLC={self.dlc} DATA=[{hexdata}])"


class MCP2515:
    def __init__(self, bus=0, device=0, speed_hz=1_000_000, osc_mhz=8, bitrate=500000):
        """
        bus/device: SPI bus/CE line (bus=0, device=0 -> /dev/spidev0.0)
        speed_hz:   SPI clock speed. Keep <= 10 MHz; 1 MHz is a safe start.
        osc_mhz:    Crystal/oscillator frequency on your MCP2515 module.
        bitrate:    Desired CAN bus bitrate (must match STM32 side).
        """
        if osc_mhz != 8:
            raise ValueError(
                "Only the 8 MHz timing table is included. Add your own "
                "CNF1/CNF2/CNF3 values to a BITRATE_8MHZ-style table in "
                "mcp2515_regs.py if your module uses a different crystal."
            )
        if bitrate not in BITRATE_8MHZ:
            raise ValueError(f"No timing entry for {bitrate} bps at 8 MHz")

        self.cnf1, self.cnf2, self.cnf3 = BITRATE_8MHZ[bitrate]

        self.spi = spidev.SpiDev()
        self.spi.open(bus, device)
        self.spi.max_speed_hz = speed_hz
        self.spi.mode = 0b00  # MCP2515 requires SPI mode 0

    # ---------------- Low-level SPI helpers ----------------
    def reset(self):
        """Issue the SPI RESET instruction; puts the chip in config mode."""
        self.spi.xfer2([INSTR_RESET])
        time.sleep(0.01)  # datasheet: allow >2us, we give it more margin

    def read_register(self, addr):
        resp = self.spi.xfer2([INSTR_READ, addr, 0x00])
        return resp[2]

    def read_registers(self, addr, n):
        resp = self.spi.xfer2([INSTR_READ, addr] + [0x00] * n)
        return resp[2:]

    def write_register(self, addr, value):
        self.spi.xfer2([INSTR_WRITE, addr, value])

    def write_registers(self, addr, values):
        self.spi.xfer2([INSTR_WRITE, addr] + list(values))

    def bit_modify(self, addr, mask, value):
        """Only works on a subset of registers per datasheet (CANCTRL,
        CANINTF, RXBnCTRL, etc). Not valid on every register."""
        self.spi.xfer2([INSTR_BIT_MODIFY, addr, mask, value])

    def read_status(self):
        return self.spi.xfer2([INSTR_READ_STATUS, 0x00])[1]

    def request_to_send(self, txb=0):
        self.spi.xfer2([INSTR_RTS | (1 << txb)])

    # ---------------- Mode control ----------------
    def set_mode(self, mode):
        self.bit_modify(CANCTRL, MODE_MASK, mode)
        # Poll CANSTAT until the chip confirms the mode switch
        deadline = time.time() + 0.5
        while time.time() < deadline:
            if (self.read_register(CANSTAT) & OPMOD_MASK) == mode:
                return
            time.sleep(0.001)
        raise TimeoutError("MCP2515 did not switch mode in time")

    # ---------------- Initialization ----------------
    def begin(self):
        """Full chip init: reset, configure bit timing, open RX filters,
        drop into normal mode."""
        self.reset()
        self.set_mode(MODE_CONFIG)

        # Bit timing
        self.write_register(CNF1, self.cnf1)
        self.write_register(CNF2, self.cnf2)
        self.write_register(CNF3, self.cnf3)

        # Disable interrupts (we'll poll CANINTF instead of using the
        # hardware INT pin, to keep this example self-contained)
        self.write_register(CANINTE, 0x00)

        # Accept every message on both receive buffers, no filtering.
        # RXM<1:0> = 11 disables mask/filter checking entirely.
        self.write_register(RXB0CTRL, RXB0CTRL_RXM_ANY | RXB0CTRL_BUKT)
        self.write_register(RXB1CTRL, RXB1CTRL_RXM_ANY)

        # Clear any stale interrupt flags
        self.write_register(CANINTF, 0x00)

        self.set_mode(MODE_NORMAL)

    # ---------------- Receiving ----------------
    def _read_rx_buffer(self, buffer_num):
        """Read a full frame out of RXB0 or RXB1."""
        instr = INSTR_READ_RXB0 if buffer_num == 0 else INSTR_READ_RXB1
        # SIDH, SIDL, EID8, EID0, DLC, D0..D7 (13 bytes after the opcode)
        resp = self.spi.xfer2([instr] + [0x00] * 13)
        sidh, sidl, eid8, eid0, dlc = resp[1:6]
        data = resp[6:6 + (dlc & 0x0F)]

        extended = bool(sidl & 0x08)
        if extended:
            arb_id = (sidh << 21) | ((sidl & 0xE0) << 13) | \
                     ((sidl & 0x03) << 16) | (eid8 << 8) | eid0
        else:
            arb_id = (sidh << 3) | (sidl >> 5)

        return CANFrame(arb_id, data, dlc=dlc & 0x0F, extended=extended)

    def read_pending_frame(self):
        """Return one CANFrame if available, else None (non-blocking)."""
        status = self.read_status()
        # READ STATUS bit layout: bit0=RX0IF, bit1=RX1IF (see datasheet
        # Table 12-4 for the rest, not needed here)
        if status & 0x01:
            frame = self._read_rx_buffer(0)
            self.bit_modify(CANINTF, RX0IF, 0x00)
            return frame
        if status & 0x02:
            frame = self._read_rx_buffer(1)
            self.bit_modify(CANINTF, RX1IF, 0x00)
            return frame
        return None

    def recv(self, poll_interval=0.001, timeout=None):
        """Blocking receive: polls until a frame arrives or timeout (s)
        elapses. Returns a CANFrame, or None on timeout."""
        deadline = None if timeout is None else time.time() + timeout
        while True:
            frame = self.read_pending_frame()
            if frame is not None:
                return frame
            if deadline is not None and time.time() > deadline:
                return None
            time.sleep(poll_interval)

    # ---------------- Sending (bonus, not required for your gateway) ----
    def send(self, arb_id, data, extended=False):
        data = list(data)[:8]
        dlc = len(data)

        if extended:
            sidh = (arb_id >> 21) & 0xFF
            sidl = (((arb_id >> 18) & 0x07) << 5) | 0x08 | ((arb_id >> 16) & 0x03)
            eid8 = (arb_id >> 8) & 0xFF
            eid0 = arb_id & 0xFF
        else:
            sidh = (arb_id >> 3) & 0xFF
            sidl = (arb_id & 0x07) << 5
            eid8 = 0x00
            eid0 = 0x00

        payload = [sidh, sidl, eid8, eid0, dlc] + data
        self.spi.xfer2([INSTR_LOAD_TXB0] + payload)
        self.request_to_send(txb=0)

    def close(self):
        self.spi.close()
