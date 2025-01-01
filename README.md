# OBD2 BLE Display

## Overview
This project is a Bluetooth Low Energy (BLE) interface for reading real-time vehicle telemetry data from an OBD2 adapter and displaying it on an OLED screen. It connects to a VEEPEAK OBD2 adapter over BLE, retrieves temperature data such as oil and coolant temperatures, and updates the display in real-time.

---

## Features
- Establishes a BLE connection with the VEEPEAK OBD2 adapter.
- Sends and receives OBD2 commands in real-time.
- Displays oil and coolant temperatures on an OLED screen.
- Automatically processes and cleans up BLE notifications from the adapter.
- Simple and extendable codebase.

---

## Hardware Requirements
- **OBD2 Adapter**: VEEPEAK BLE OBD2 Adapter (or a compatible BLE-based OBD2 reader).
- **Microcontroller**: ESP32 or similar with BLE support.
- **Display**: SSD1306-based OLED display (128x32 resolution).
- **Power Supply**: USB or a compatible power source for your microcontroller.

---

## Software Requirements

### Tools
- **Arduino IDE**: Latest version of the Arduino IDE.

### Libraries
The following libraries need to be installed in the Arduino IDE:
- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit SSD1306 Library](https://github.com/adafruit/Adafruit_SSD1306)
- [ESP32 BLE Arduino](https://github.com/nkolban/ESP32_BLE_Arduino)

---

## Wiring Diagram

| **Microcontroller Pin** | **OLED Pin** |
|--------------------------|--------------|
| 3.3V                     | VCC          |
| GND                      | GND          |
| SDA (e.g., GPIO21)       | SDA          |
| SCL (e.g., GPIO22)       | SCL          |

---

## Usage

1. Once powered on, the ESP32 will scan for BLE devices.
2. Upon detecting the `VEEPEAK` OBD2 adapter, it will attempt to connect.
3. Commands are sent to initialize the OBD2 communication protocol.
4. Data such as oil and coolant temperatures are queried periodically and displayed on the OLED screen.


---

## Troubleshooting

- **No BLE Device Found**: Ensure the OBD2 adapter is powered and within range.
- **Display Not Working**: Check the wiring and ensure the correct I2C address is set in the code.
- **BLE Connection Fails**: Verify the adapter is compatible and not connected to another device.

---

## Example Output

### OLED Display Example:
```
Oil     Water
75       90
```

---

## License
This project is licensed under the [MIT License](LICENSE).

---

## Author
Created by **Linus Kemper**. Contributions are welcome! Feel free to open issues or submit pull requests.