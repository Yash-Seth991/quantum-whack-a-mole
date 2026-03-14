import serial
import time
from qiskit import QuantumCircuit, transpile
from qiskit_aer import AerSimulator

# -----------------------------
# Serial port configuration
# -----------------------------
arduino_port = "COM6" # Replace this with your actual COM port
baud_rate = 9600

try:
    ser = serial.Serial(arduino_port, baud_rate, timeout=1)
    time.sleep(2)  # wait for Arduino to reset
except Exception as e:
    print("Error opening serial port:", e)
    exit()

# -----------------------------
# Quantum Random Mole Generator
# -----------------------------
def quantum_random_mole():
    qc = QuantumCircuit(3, 3)
    qc.h([0, 1, 2])
    qc.measure([0, 1, 2], [0, 1, 2])
    simulator = AerSimulator()
    transpiled_qc = transpile(qc, simulator)
    result = simulator.run(transpiled_qc, shots=1).result()
    counts = result.get_counts(qc)
    bitstring = list(counts.keys())[0]
    return int(bitstring, 2) % 5

# -----------------------------
# Speed control variables
# -----------------------------
base_delay = 1.0  # starting delay in seconds
min_delay = 0.15  # minimum possible delay
current_score = 0

# -----------------------------
# Main loop
# -----------------------------
print("Quantum Whack-a-Mole Serial Sender running...")

while True:
    # Check if Arduino sent a new score
    while ser.in_waiting:
        line = ser.readline().decode().strip()
        if line.startswith("SCORE:"):
            current_score = int(line.split(":")[1])
            print(f"Current Score: {current_score}")

    # Generate a new mole
    mole_number = quantum_random_mole()
    print(f"Generated Mole: {mole_number}")

    # Send mole number to Arduino
    try:
        ser.write(f"{mole_number}\n".encode())
    except Exception as e:
        print("Error sending data:", e)

    # Delay based on Arduino score
    delay = max(min_delay, base_delay - current_score * 0.05)
    time.sleep(delay)
