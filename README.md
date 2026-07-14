# CAN Sensor Telemetry Network

STM32-based sensor telemetry ECUs publishing periodic data over a shared CAN
bus, with a Raspberry Pi 5 acting as a CAN-to-Ethernet gateway. Built against
the "Sensor Telemetry Node ECU" project SOP in `docs/Project_SOP.pdf`.

## Hardware

- 5x STM32 Nucleo-F446RE, each with an SN65HVD230 CAN transceiver, wired onto
  one shared CAN bus (500 kbps)
- 1x MPU6050 (I2C) — accelerometer + gyroscope
- 1x DHT11 — temperature + humidity
- 1x DS18B20 (1-Wire) — temperature
- 1x Raspberry Pi 5 with an MCP2515 CAN controller (SPI, direct driver — no
  SocketCAN) acting as a CAN → UDP Ethernet gateway

## Repo layout

```
firmware/
  mpu6050_node/     STM32 node: IMU (accel + gyro), CAN IDs 0x100 / 0x101
  dht11_node/       STM32 node: temp + humidity, CAN ID 0x201
  ds18b20_node/     STM32 node: temperature, CAN ID 0x301
  speed_node/       STM32 node: simulated speed value, CAN ID 0x401
  receiver_node/    STM32 node: listens to the whole bus, prints status over UART
gateway/            Raspberry Pi: MCP2515 SPI driver + CAN->UDP gateway script
dashboard/          Two independent visualization options (see below)
docs/               Original project SOP
```

## CAN frame map

| CAN ID | Node     | DLC | Bytes                                  |
|--------|----------|-----|-----------------------------------------|
| 0x100  | MPU6050  | 6   | AX_hi, AX_lo, AY_hi, AY_lo, AZ_hi, AZ_lo (int16, x100 scale, m/s²) |
| 0x101  | MPU6050  | 6   | GX_hi, GX_lo, GY_hi, GY_lo, GZ_hi, GZ_lo (int16, x100 scale, deg/s) |
| 0x201  | DHT11    | 2   | Temp (°C, uint8), Humidity (%, uint8)  |
| 0x301  | DS18B20  | 3   | Temp_hi, Temp_lo (int16, x100 scale, °C), Fault (0/1) |
| 0x401  | Speed    | 2   | Speed_hi, Speed_lo (uint16, km/h)      |

**Known gap (not yet fixed in this snapshot):** the MPU6050 node does not
transmit a fault byte, and the DHT11 node computes a fault flag but never
places it in `TxData`, so it's silently dropped. The Receiver node currently
infers node health only from bus-silence timeouts (>1s since last frame),
not from sensor-reported faults. Happy to patch this if/when you want it —
ask and I'll wire it through firmware + Receiver decode together.

## Data path

Two independent, parallel ways to view the telemetry:

1. **STM32 Receiver node → USB-UART → `dashboard/can_dashboard_v9.html`**
   (Chrome/Edge, Web Serial API — connect directly, no Pi involved)
2. **Raspberry Pi 5 (MCP2515) → UDP/Ethernet → `dashboard/Receiver.py`**
   (Tkinter GUI on your laptop, listens on UDP port 5000)

## Building the firmware

Each folder under `firmware/` is a `Core/Src` + `Core/Inc` pair meant to be
dropped into an STM32CubeIDE project generated for the Nucleo-F446RE with
the matching peripherals enabled (CAN1, plus I2C1 for the MPU6050/receiver
nodes, or TIM2 for the bit-banged DHT11/DS18B20 nodes). Regenerate the
CubeMX boilerplate for each node, then replace `Core/Src/main.c` (and
add the sensor driver files) with the versions here.

## Running the gateway (Raspberry Pi)

```bash
pip3 install spidev
# edit gateway/config.py -> set LAPTOP_IP to your laptop's IP
python3 gateway/can_udp_gateway.py
```

## Running the Tkinter dashboard (laptop)

```bash
python3 dashboard/Receiver.py
```

## Opening the web dashboard

Open `dashboard/can_dashboard_v9.html` in Chrome or Edge, click connect, and
select the Receiver node's serial port.

## License

MIT — see `LICENSE`.
