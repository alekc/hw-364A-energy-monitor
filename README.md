# Energy Monitor

This is an ESP8266-based IoT energy monitoring display project. It connects to an MQTT broker to fetch real-time power data from a Victron system, as well as a custom energy monitoring topic. It then displays this information on an SSD1306 OLED display.

## Features

- **Wi-Fi & MQTT Integration:** Connects seamlessly to your local network and MQTT broker.
- **Victron System Data:** Subscribes to Victron MQTT topics to retrieve:
  - Grid Power
  - Battery State of Charge (SOC)
- **Home & Solar Data:** Subscribes to a custom MQTT topic (`home/energy/monitor`) to track:
  - Overall home power usage
  - Solar power generation
- **OLED Interface:** Displays data on an I2C OLED screen (128x64 pixels). The screen automatically toggles every 5 seconds between two views:
  - **Inverter Screen:** Shows Grid Power (W) and Battery SOC (%).
  - **Home / Solar Screen:** Shows Home Power (W) and Solar Power (W).
- **Graceful Fallbacks:** Shows "NO DATA" if no MQTT messages are received within a 30-second window. Details device uptime at the bottom of the screen.

## Hardware Required

- ESP8266 microcontroller (e.g., NodeMCU v2)
- SSD1306 OLED display (I2C)
  - **SDA** connects to pin D6 (GPIO 14)
  - **SCL** connects to pin D5 (GPIO 12)

## Software Dependencies

This project is built using [PlatformIO](https://platformio.org/) and the Arduino framework. Key libraries used include:
- `olikraus/U8g2` (or `Adafruit SSD1306` + `Adafruit GFX Library`) for driving the OLED display.
- `knolleary/PubSubClient` for handling MQTT communication.
- `bblanchon/ArduinoJson` for extracting values from the MQTT JSON payloads.

## Setup Instructions

1. **Clone the repository.**
2. **Configure Secrets:**
   - Copy `include/secrets.example.h` to `include/secrets.h`.
   - Open `include/secrets.h` and populate it with your specific Wi-Fi credentials, MQTT server details, and `PORTAL_ID` for your Victron installation.
3. **Build and Upload:**
   - Open the project in VS Code with the PlatformIO extension installed.
   - Build the project and upload it to your ESP8266 board.
