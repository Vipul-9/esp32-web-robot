# Web-Controlled Mobile Robot (ESP32)

A two-wheel robot driven from any phone or laptop browser through a control page hosted on the **ESP32**. No app is needed.

## Features
- **Press-and-hold controls:** forward, back, spin left, spin right; release to stop
- **Speed slider** using PWM
- **Keyboard control** with the arrow keys on a laptop
- **Failsafe:** the motors stop if no command arrives for 500 ms, for example if Wi-Fi drops
- **Wi-Fi modes:** joins your network, or falls back to its own hotspot (`ESP32-Robot`)

```
Browser → Wi-Fi → ESP32 web server → L298N motor driver → 2 DC motors
```

## Hardware
ESP32 dev board, L298N motor driver, 2 DC geared motors with wheels, chassis with a caster, and a motor battery pack (7–12 V)

## Wiring
| L298N | ESP32 |
|---|---|
| ENA | GPIO 14 |
| IN1 | GPIO 27 |
| IN2 | GPIO 26 |
| ENB | GPIO 32 |
| IN3 | GPIO 25 |
| IN4 | GPIO 33 |
| GND | GND (common) |

Remove the ENA/ENB jumpers on the L298N so the ESP32 controls speed.

## How to implement

**1. Build the robot**
- Mount the two geared motors and the caster on the chassis, then fix the L298N and the ESP32 on top.
- Connect the motors to the L298N **OUT1/OUT2** (left) and **OUT3/OUT4** (right) terminals.
- Battery (7–12 V) **+** goes to L298N **12V** and **−** to **GND**. Power the ESP32 from the L298N **5V** pin into **VIN**, or over USB while testing.
- Wire the control pins as in the table above, and **join all grounds**.
- Remove the ENA/ENB jumpers.

**2. Set up the Arduino IDE**
- File → Preferences → *Additional boards manager URLs*: add `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
- Tools → Board → Boards Manager: install **esp32 by Espressif** (2.0.11 or newer).
- No extra libraries are needed: `WiFi` and `WebServer` come with the core.

**3. Configure and upload**
- Open `firmware/esp32_web_robot/esp32_web_robot.ino`.
- To join your Wi-Fi, set `WIFI_SSID` and `WIFI_PASS`. Leave them empty for hotspot mode.
- Select **ESP32 Dev Module** and the right COM port, then upload. If the upload stalls at *Connecting…*, hold **BOOT**.

**4. Drive it**
- Open the Serial Monitor at **115200** baud to see the IP address.
- **Your Wi-Fi:** open that IP from a phone or laptop on the same network.
- **Hotspot:** connect to `ESP32-Robot` (password `robot1234`) and open `http://192.168.4.1`.
- Hold a button to move and release to stop. Arrow keys also work on a laptop, and the slider sets the speed.

**Troubleshooting**
- *A wheel spins the wrong way:* swap that motor's two wires.
- *Robot resets when the motors start:* the battery is too weak, or the grounds aren't shared.
- *Page won't load:* make sure the device is on the same network or connected to the hotspot.

## Endpoints
| Route | Action |
|---|---|
| `/` | Control page |
| `/cmd?d=F\|B\|L\|R\|S` | Drive command |
| `/speed?v=0-255` | Set PWM speed |

## Contributing
I'm open to open-source contributions and collaboration. Issues and pull requests are welcome.
You can reach me at **vipulatluri98@gmail.com**.
