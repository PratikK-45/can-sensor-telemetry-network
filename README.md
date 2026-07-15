# 🚗 CAN Sensor Telemetry Network
A real-time automotive communication system that simulates multiple (ECUs) communicating over a Controller Area Network (CAN) Bus. The project collects sensor data from different ECUs, transmits it to a central receiver, detects abnormal CAN traffic, and forwards the received data over Ethernet using UDP for remote monitoring.

---

# 📖 Overview

This project implements a distributed **Automotive Sensor Telemetry Network** that simulates how multiple Electronic Control Units (ECUs) communicate in modern vehicles.

Five STM32F446RE-based ECUs acquire real-time sensor data and exchange telemetry over the **Controller Area Network (CAN)**. A dedicated Receiver ECU collects and validates the incoming CAN messages before forwarding them to a **Raspberry Pi 5**.

Using an **MCP2515 CAN Controller**, the Raspberry Pi acts as a **CAN-to-Ethernet Gateway**, converting CAN telemetry into UDP packets and transmitting them over Ethernet to a remote PC.

The received telemetry is visualized through a real-time web dashboard, providing live sensor monitoring, system health, fault indication, and data visualization.

---

# 🎯 Problem Statement

Design and implement a distributed sensor telemetry subsystem capable of acquiring physical sensor data from multiple embedded nodes and periodically publishing the telemetry to an external monitoring system using a fixed and reliable communication format.

The system should operate continuously, tolerate sensor failures without interrupting communication, and emulate the architecture used in modern automotive embedded systems.

---

# 🚀 Project Objectives

- Develop multiple STM32-based Sensor ECUs
- Acquire real-time sensor data
- Transmit periodic telemetry over CAN Bus
- Implement a dedicated Receiver ECU
- Build a CAN-to-Ethernet Gateway using Raspberry Pi 5
- Transfer telemetry using UDP over Ethernet
- Design a real-time monitoring dashboard
- Implement sensor fault detection and reporting
- Demonstrate continuous system operation without interruption

---

# 🏗 System Architecture

```

MPU6050 ECU
│
├──────────────┐
│
DHT11 ECU
│
├──────────────┤
│
DS18B20 ECU
│
├──────────────┤
│
Speed ECU
│
└──────────────┘

↓

CAN BUS

↓

Receiver ECU
(STM32F446RE)

↓

SPI

↓

MCP2515 CAN Controller

↓

Raspberry Pi 5
(CAN-to-Ethernet Gateway)

↓

UDP over Ethernet

↓

Remote PC

↓

Python Receiver

↓

Web Dashboard

```

---

# ⚙ Hardware Components

| Component | Purpose |
|------------|----------|
| STM32F446RE (5 Boards) | Sensor ECUs + Receiver ECU |
| MPU6050 | Accelerometer & Gyroscope |
| DHT11 | Temperature & Humidity |
| DS18B20 | Digital Temperature Sensor |
| MCP2515 | CAN Controller |
| Raspberry Pi 5 | CAN-to-Ethernet Gateway |
| SN65HVD230 | CAN Transceiver |
| Ethernet Network | Data Transmission |
| PC | Dashboard & Monitoring |

---

# 📡 ECU Configuration

| ECU | Function |
|------|----------|
| ECU-1 | MPU6050 Sensor Node |
| ECU-2 | DHT11 Sensor Node |
| ECU-3 | DS18B20 Sensor Node |
| ECU-4 | Speed Data Node |
| ECU-5 | Receiver ECU |

---

# 📊 Sensor Telemetry

The system periodically transmits:

- Accelerometer X
- Accelerometer Y
- Accelerometer Z
- Gyroscope X
- Gyroscope Y
- Gyroscope Z
- Temperature
- Humidity
- DS18B20 Temperature
- Vehicle Speed
- Sensor Status
- Fault Status

---

# 📦 Telemetry Features

✔ Fixed Transmission Format

✔ Periodic Data Publishing

✔ Consistent Data Scaling

✔ Continuous Transmission

✔ Automatic Startup

✔ Multi-node Communication

✔ Real-time Monitoring

---

# ⚠ Fault Behaviour

The system implements fault-tolerant telemetry.

If any sensor fails,

- Telemetry continues uninterrupted.
- Remaining sensor values are transmitted normally.
- A fault flag is added to the telemetry frame.
- The dashboard immediately indicates the failed sensor.

Example:

```

DS18B20
Status : SENSOR FAULT

```

The system never hangs or stops transmitting due to a sensor failure.

---

# 🌐 CAN-to-Ethernet Gateway

The Raspberry Pi 5 functions as the communication gateway.

Responsibilities include:

- Reading CAN frames through MCP2515
- Decoding telemetry
- Creating UDP packets
- Transmitting telemetry over Ethernet
- Maintaining continuous communication with the monitoring PC

---

# 📈 Dashboard Features

The dashboard provides:

- Live CAN Stream
- Sensor Status
- Vehicle Speed
- Accelerometer Visualization
- Gyroscope Visualization
- Motion Radar Graph
- Speed History
- Temperature History
- Humidity Display
- Sensor Fault Alerts
- Connection Status
- CSV Export

---

# 📂 Repository Structure

```

can-sensor-telemetry-network/

├── firmware/
│
├── mpu6050_node/
├── dht11_node/
├── ds18b20_node/
├── speed_node/
├── receiver_node/
│
├── gateway/
│
├── can_udp_gateway.py
├── mcp2515.py
├── config.py
│
├── dashboard/
│
├── Receiver.py
├── can_dashboard_v9.html
│
├── block_diagram.png
│
├── README.md
└── LICENSE

```

---

## Dashboard

- Live CAN Stream
- Speed Graph
- Temperature Graph
- Motion Snapshot
- Sensor Health
- Fault Alerts

---

# 🧪 Testing

The project was validated for:

- Multi-node CAN Communication
- Sensor Acquisition
- Continuous Telemetry
- Receiver Validation
- Gateway Communication
- UDP Transmission
- Dashboard Visualization
- Fault Behaviour
- Continuous Runtime

---

# 🎓 Learning Outcomes

Through this project, the following concepts were implemented:

- Embedded C Programming
- STM32 HAL Development
- CAN Protocol
- Multi-ECU Communication
- Sensor Interfacing
- Raspberry Pi Integration
- MCP2515 CAN Controller
- UDP Networking
- Ethernet Communication
- Real-Time Dashboard Development
- Fault Detection
- Automotive Embedded System Design

---
