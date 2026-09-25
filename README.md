# Arduino Pressure Sensor Tester

Arduino-based pressure sensor comparison and characterization tool using a BMP390 reference sensor, ADS1115 ADC, OLED display, and surplus pressure sensors.

The project is being developed incrementally, starting with a bench-top pressure tracker and eventually evolving into a controlled pressure chamber test system with printed test results.

## Current Status

The current prototype uses:

- Arduino Uno R3
- BMP390 digital barometric pressure sensor as the reference sensor
- ADS1115 16-bit ADC for differential analog sensor measurements
- 128x64 I2C OLED display
- U8g2 display library using page-buffer mode to conserve Uno SRAM

The current firmware:

- Reads pressure and temperature from the BMP390
- Reads the ADS1115 differential input
- Converts the analog sensor reading into an estimated pressure value
- Displays pressure information on the OLED
- Outputs diagnostic data over the Arduino Serial Monitor
- Detects when the analog sensor does not appear to be connected

The analog pressure sensor transfer function currently in the code is provisional and will be updated once the exact sensor documentation is confirmed.

## Repository Structure

    arduino-pressure-sensor-tester/
    ├── README.md
    ├── LICENSE
    ├── .gitignore
    └── pressure_sensor_tracker/
        └── pressure_sensor_tracker.ino

## Hardware

### Arduino

- Arduino Uno R3

### Reference Sensor

- BMP390 barometric pressure sensor
- I2C interface
- Current I2C address: `0x77`

### Analog-to-Digital Converter

- ADS1115
- 16-bit ADC
- I2C interface
- Current I2C address: `0x48`
- Differential measurement using A0 and A1

### OLED Display

- 128x64 I2C OLED
- Current I2C address: `0x3C`
- Driven using U8g2 page-buffer mode

### Analog Pressure Sensor

The project is intended to test surplus differential-output pressure sensors.

The exact sensor model and transfer function are still being verified. The current firmware contains provisional conversion values and should not yet be treated as calibrated measurement code.

## Wiring

All three I2C devices share the Arduino Uno I2C bus.

### I2C Bus

| Arduino Uno | Device Connection |
|---|---|
| A4 | SDA |
| A5 | SCL |
| 5V | Device power where supported |
| GND | Common ground |

### ADS1115

| ADS1115 Pin | Connection |
|---|---|
| VDD | 5V |
| GND | GND |
| SDA | Arduino A4 |
| SCL | Arduino A5 |
| ADDR | GND |
| A0 | Analog sensor +Vout |
| A1 | Analog sensor -Vout |
| A2 | Not currently used |
| A3 | Not currently used |

### BMP390

| BMP390 Pin | Connection |
|---|---|
| VCC / VIN | 5V on the current breakout |
| GND | GND |
| SDA | Arduino A4 |
| SCL | Arduino A5 |

### OLED

| OLED Pin | Connection |
|---|---|
| VCC | 5V on the current module |
| GND | GND |
| SDA | Arduino A4 |
| SCL | Arduino A5 |

## Required Arduino Libraries

Install the following libraries through the Arduino IDE Library Manager:

- U8g2
- Adafruit BMP3XX Library
- Adafruit ADS1X15

Dependencies required by those libraries may also be installed automatically by the Arduino IDE.

## Serial Output

The sketch outputs diagnostic information at 115200 baud.

Example output:

    BMP390: 1007.42 hPa  Temp: 21.3 C | IDEX: NOT CONNECTED | ADC: 0.015 mV

Once the analog pressure sensor is connected and its transfer function is verified, the output will also include its estimated pressure and the difference from the BMP390 reference reading.

## Planned Development

Planned stages include:

- Verify the exact analog pressure sensor model and specifications
- Finalize the voltage-to-pressure conversion
- Compare analog sensor output against the BMP390 over time
- Track measurement count, pressure range, and difference range
- Add running statistics
- Add controlled testing using a pressure chamber
- Add guided pressure sweep testing
- Add date and time support
- Add Zebra printer output using ZPL
- Generate printed test or characterization labels
- Refine the system for a hands-on Maker Faire exhibit

## Project Goal

The long-term goal is to build an interactive pressure sensor characterization system.

A user will be able to change the pressure inside a chamber, compare a sensor under test against a known reference sensor, review the results on the display, and eventually print a small label containing the test results.

The system is intended as an educational and experimental tool rather than certified calibration equipment.

## License

This project is released under the MIT License.
