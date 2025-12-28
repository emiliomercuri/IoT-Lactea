import serial

ser = serial.Serial("/dev/ttyUSB0", 9600, timeout=1)

print("Lendo CO₂ (Ctrl+C para sair)")

try:
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode().strip()
            if line.startswith("CO2:"):
                co2 = int(line.split(":")[1].strip())
                print(f"CO₂: {co2} ppm")
except KeyboardInterrupt:
    print("\nEncerrando...")
finally:
    ser.close()

