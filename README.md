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

## Setup
1. In the Arduino IDE, install the ESP32 board package (version 2.0.11 or newer).
2. Open `firmware/esp32_web_robot/esp32_web_robot.ino`.
3. Optional: set `WIFI_SSID` and `WIFI_PASS` to join your network.
4. Upload it, then open the IP address shown in the Serial Monitor (115200 baud).
   - In hotspot mode, connect to `ESP32-Robot` (password `robot1234`) and open `http://192.168.4.1`.

## Endpoints
| Route | Action |
|---|---|
| `/` | Control page |
| `/cmd?d=F\|B\|L\|R\|S` | Drive command |
| `/speed?v=0-255` | Set PWM speed |
