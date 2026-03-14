# 🎮 Quantum Whack-A-Mole

A hardware-software hybrid game that replaces traditional pseudo-random number generation with **quantum randomness** — powered by a 3-qubit circuit running on IBM's Qiskit Aer simulator, with results sent in real time to an Arduino.

> *"What if the mole knew quantum mechanics?"*

---

## 🧠 The Idea

Every Whack-A-Mole game ever made uses a classical pseudo-random number generator (PRNG) to decide which hole the mole pops out of. PRNGs are deterministic — given the same seed, they produce the same sequence every time. That's not true randomness.

This project replaces the PRNG with a **3-qubit quantum circuit**. Each qubit is placed in superposition via a Hadamard gate, then measured. The resulting 3-bit binary string (000–111) is mapped to one of 5 mole positions — genuinely probabilistic in a way no classical computer can replicate. That result is sent over serial to an Arduino which runs the full game on an OLED display.

---

## ⚙️ How It Works

```
[Python — quantum_mole.py]
  → builds 3-qubit circuit (Hadamard on each qubit)
  → measures via Aer simulator → 3-bit result (e.g. "011")
  → maps to mole index 0–4 (int(bitstring, 2) % 5)
  → sends index over serial (USB) to Arduino

[Arduino — whack_a_mole.ino]
  → reads mole index from serial
  → animates mole pop-up on 128x64 OLED display
  → player uses joystick to whack the correct mole
  → score tracked; game speeds up as score increases
  → sends score back to Python to adjust spawn delay
```

---

## 🔌 Hot-Swappable Quantum Backend

The quantum backend is modular. By default it uses the **Aer local simulator** (free, offline). To run on real IBM Quantum hardware, swap two lines:

```python
# Default — local simulator, free and instant
simulator = AerSimulator()

# Real quantum hardware (requires IBM Quantum account)
# from qiskit_ibm_runtime import QiskitRuntimeService
# service = QiskitRuntimeService()
# backend = service.least_busy(operational=True, simulator=False)
```

> Real quantum backends charge by compute time, so the simulator is used by default. The circuit and serial logic are identical either way.

---

## 🛠️ Tech Stack

| Layer | Technology |
|---|---|
| Quantum circuit | Qiskit (Python) — 3-qubit Hadamard + measurement |
| Quantum backend | IBM Aer Simulator (hot-swappable to real IBM QPU) |
| Serial bridge | PySerial (Python) |
| Embedded controller | Arduino (C++) |
| Display | 128x64 OLED (Adafruit SSD1306) |
| Input | Joystick (X/Y analog + button) |
| Audio | Buzzer (tone feedback on hit/miss/spawn) |

---

## 📦 Setup & Installation

### Hardware Required
- Arduino Uno or Nano
- 128x64 OLED display (SSD1306, I2C)
- Joystick module (analog X/Y + button)
- Passive buzzer
- Jumper wires

### Wiring

| Component | Arduino Pin |
|---|---|
| OLED SDA | A4 |
| OLED SCL | A5 |
| OLED VCC | 3.3V |
| OLED GND | GND |
| Joystick VRX | A6 |
| Joystick VRY | A7 |
| Joystick SW | D3 |
| Buzzer | D9 |

### Arduino Libraries Required
Install via Arduino IDE Library Manager:
- `Adafruit SSD1306`
- `Adafruit GFX Library`

### Python Dependencies
```bash
pip install qiskit qiskit-aer pyserial
```

### Upload & Run
1. Open `whack_a_mole.ino` in Arduino IDE
2. Select your board and port, then upload
3. Update the COM port in `quantum_mole.py`:
```python
arduino_port = "COM6"  # Windows — change to match your port
# arduino_port = "/dev/ttyUSB0"  # Linux/Mac
```
4. Run the Python script:
```bash
python quantum_mole.py
```

---

## 🎮 Gameplay

- The mole pops up on the OLED display in one of 5 positions
- Use the **joystick** to select the correct mole position and **press the button** to whack it
- Hit the right mole → score goes up
- Miss or hit the wrong mole → lose a life (3 lives total)
- Game speeds up as your score increases — both mole visibility duration and spawn delay decrease with every point

---

## 📰 Write-Up

This project was written up and published on Medium:
- [Whack-A-Mole Meets Quantum Computing](https://medium.com/@yashseth991)
- [The Closest I Got to Having a Quantum Computer at Home](https://medium.com/@yashseth991)

---

## 📁 File Structure

```
quantum-whack-a-mole/
├── whack_a_mole.ino     # Arduino sketch — game logic, OLED, joystick
├── quantum_mole.py      # Python — quantum circuit + serial bridge
└── README.md
```

---

## 👤 Author

**Yash Seth**
[LinkedIn](https://linkedin.com/in/yash-seth-aab34b340) · [Medium](https://medium.com/@yashseth991) · [GitHub](https://github.com/Yash-Seth991)
