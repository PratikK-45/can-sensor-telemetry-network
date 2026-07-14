import socket
import threading
import tkinter as tk
from tkinter import ttk

# -----------------------------
# UDP Configuration
# -----------------------------
HOST = "0.0.0.0"
PORT = 5000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((HOST, PORT))

# -----------------------------
# GUI
# -----------------------------
root = tk.Tk()
root.title("CAN Sensor Dashboard")
root.geometry("650x500")
root.resizable(False, False)

title = tk.Label(root, text="CAN SENSOR DASHBOARD", font=("Arial", 18, "bold"), fg="blue")
title.pack(pady=10)

frame = ttk.Frame(root, padding=20)
frame.pack(fill="both", expand=True)

# -----------------------------
# Variables
# -----------------------------
accX = tk.StringVar(value="0.00 g")
accY = tk.StringVar(value="0.00 g")
accZ = tk.StringVar(value="0.00 g")

gyroX = tk.StringVar(value="0.00")
gyroY = tk.StringVar(value="0.00")
gyroZ = tk.StringVar(value="0.00")

dhtTemp = tk.StringVar(value="0 \u00b0C")
humidity = tk.StringVar(value="0 %")

dsTemp = tk.StringVar(value="0.00 \u00b0C")

speed = tk.StringVar(value="0 km/h")
status = tk.StringVar(value="Waiting...")

# -----------------------------
# Frames
# -----------------------------
accFrame = ttk.LabelFrame(frame, text="Accelerometer")
accFrame.grid(row=0, column=0, padx=10, pady=10)

gyroFrame = ttk.LabelFrame(frame, text="Gyroscope")
gyroFrame.grid(row=0, column=1, padx=10, pady=10)

dhtFrame = ttk.LabelFrame(frame, text="DHT11")
dhtFrame.grid(row=1, column=0, padx=10, pady=10)

dsFrame = ttk.LabelFrame(frame, text="DS18B20")
dsFrame.grid(row=1, column=1, padx=10, pady=10)

speedFrame = ttk.LabelFrame(frame, text="Speed")
speedFrame.grid(row=2, column=0, columnspan=2, padx=10, pady=10, sticky="ew")


def add_row(parent, name, var, row):
    ttk.Label(parent, text=name, width=12).grid(row=row, column=0, sticky="w")
    ttk.Label(parent, textvariable=var, font=("Arial", 11, "bold"),
              foreground="blue").grid(row=row, column=1, sticky="w")


# Accelerometer
add_row(accFrame, "X", accX, 0)
add_row(accFrame, "Y", accY, 1)
add_row(accFrame, "Z", accZ, 2)

# Gyroscope
add_row(gyroFrame, "X", gyroX, 0)
add_row(gyroFrame, "Y", gyroY, 1)
add_row(gyroFrame, "Z", gyroZ, 2)

# DHT11
add_row(dhtFrame, "Temp", dhtTemp, 0)
add_row(dhtFrame, "Humidity", humidity, 1)

# DS18B20
add_row(dsFrame, "Temp", dsTemp, 0)

# Speed
add_row(speedFrame, "Speed", speed, 0)

statusLabel = ttk.Label(root, textvariable=status, font=("Arial", 11))
statusLabel.pack()


# ------------------------------------------------
# Receiver Thread
# ------------------------------------------------
def receiver():
    while True:
        packet, addr = sock.recvfrom(1024)
        message = packet.decode().strip()
        print(message)

        try:
            # Example:
            # ID=0x100 DLC=6 DATA=00 05 ff fe 00 6a
            parts = message.split()

            # CAN ID
            can_id = int(parts[0].split("=")[1], 16)

            # DLC
            dlc = int(parts[1].split("=")[1])

            # DATA bytes
            data_list = []
            # First byte after DATA=
            data_list.append(parts[2].split("=")[1])
            # Remaining bytes
            for i in range(3, len(parts)):
                data_list.append(parts[i])

            # Convert to bytes
            data = bytes(int(x, 16) for x in data_list)

            # Verify length
            if len(data) != dlc:
                print("Length mismatch")
                continue

            # -----------------------------------
            # Accelerometer
            # -----------------------------------
            if can_id == 0x100:
                ax = int.from_bytes(data[0:2], 'big', signed=True) / 100
                ay = int.from_bytes(data[2:4], 'big', signed=True) / 100
                az = int.from_bytes(data[4:6], 'big', signed=True) / 100
                accX.set(f"{ax:.2f} g")
                accY.set(f"{ay:.2f} g")
                accZ.set(f"{az:.2f} g")

            # -----------------------------------
            # Gyroscope
            # -----------------------------------
            elif can_id == 0x101:
                gx = int.from_bytes(data[0:2], 'big', signed=True) / 100
                gy = int.from_bytes(data[2:4], 'big', signed=True) / 100
                gz = int.from_bytes(data[4:6], 'big', signed=True) / 100
                gyroX.set(f"{gx:.2f}")
                gyroY.set(f"{gy:.2f}")
                gyroZ.set(f"{gz:.2f}")

            # -----------------------------------
            # DHT11
            # -----------------------------------
            elif can_id == 0x201:
                dhtTemp.set(f"{data[0]} \u00b0C")
                humidity.set(f"{data[1]} %")

            # -----------------------------------
            # DS18B20
            # -----------------------------------
            elif can_id == 0x301:
                temp = int.from_bytes(data[0:2], 'big', signed=True) / 100
                dsTemp.set(f"{temp:.2f} \u00b0C")

            # -----------------------------------
            # Speed
            # -----------------------------------
            elif can_id == 0x401:
                spd = int.from_bytes(data[0:2], 'big')
                speed.set(f"{spd} km/h")

            status.set("Receiving CAN Frames")

        except Exception as e:
            print("ERROR:", e)


threading.Thread(target=receiver, daemon=True).start()
root.mainloop()
