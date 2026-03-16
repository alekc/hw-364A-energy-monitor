#include <Arduino.h>
#include <Wire.h>

// Choose which library to use by setting this directly
// 1 = U8g2 , 2 = Adafruit SSD1306
#define ACTIVE_LIB 1

// Include secrets
#include "secrets.h"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient);

#define MQTT_TOPIC "victron/N/" PORTAL_ID "/system/0/Ac/Grid/L1/Power"
float gridPower = 0.0;
unsigned long lastValueReceived = 0;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // We only subscribe to one topic, but we can verify it
  if (String(topic) == MQTT_TOPIC) {
    // Deserialize JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, length);

    if (!error) {
      gridPower = doc["value"] | 0.0f;
      lastValueReceived = millis();
      Serial.print("Updated Power: ");
      Serial.println(gridPower);
    } else {
      Serial.print("JSON parse failed: ");
      Serial.println(error.c_str());
    }
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SECRET_WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "EnergyMonitor-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("energyMonitor/status", "online");
      // ... and resubscribe
      mqttClient.subscribe(MQTT_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

#if ACTIVE_LIB == 1
// ---------------- U8g2 Implementation ---------------- //
#include <U8g2lib.h>

#define OLED_RESET     U8X8_PIN_NONE // Reset pin (not used)
#define OLED_SDA 14                  // D6
#define OLED_SCL 12                  // D5

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, OLED_RESET, OLED_SCL, OLED_SDA);

int c = 0;

void handle_oled(int c) {
  u8g2.clearBuffer();

  // Switch screens every 5 seconds (5000 ms)
  int currentScreen = (millis() / 5000) % 2;

  if (currentScreen == 0) {
    // ---- SCREEN 1: AC Current ----
    // Top 16 pixels (often Yellow on dual-color screens)
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 14, "AC Current");

    // Bottom 48 pixels (often Blue)
    if (millis() - lastValueReceived > 30000 || lastValueReceived == 0) {
      // Timeout or no initial data
      u8g2.setFont(u8g2_font_ncenB10_tr); // slightly smaller font for text
      u8g2.drawStr(0, 42, "NO DATA");
    } else {
      // Show Value
      char buffer[32];
      snprintf(buffer, sizeof(buffer), "%.0f W", gridPower);
      u8g2.setFont(u8g2_font_ncenB18_tr); // larger font for value
      u8g2.drawStr(0, 45, buffer);
    }
  } else {
    // ---- SCREEN 2: Placeholder ----
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 14, "System Info");

    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 42, "Screen 2...");
  }

  // Still show uptime just for stats at bottom edge
  char time_buf[20];
  snprintf(time_buf, sizeof(time_buf), "Uptime: %ds", c);
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 63, time_buf);

  u8g2.sendBuffer();
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  mqttClient.setServer(SECRET_MQTT_SERVER, 1883);
  mqttClient.setCallback(mqttCallback);

  u8g2.begin();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  handle_oled(c);
  c++;
  delay(1000);
}

#elif ACTIVE_LIB == 2
// ---------------- Adafruit SSD1306 Implementation ---------------- //
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1   // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // If not work please scan the bus
#define OLED_SDA 14         // D6
#define OLED_SCL 12         // D5

Adafruit_SSD1306 *display;

int c = 0;

void handle_oled(int c) {
  display->clearDisplay();

  // Switch screens every 5 seconds (5000 ms)
  int currentScreen = (millis() / 5000) % 2;

  display->setTextColor(SSD1306_WHITE);

  if (currentScreen == 0) {
    // ---- SCREEN 1: AC Current ----
    // Top 16 pixels (often Yellow on dual-color screens)
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->println("AC Current");

    // Draw a dividing line between yellow and blue sections (16px split)
    display->drawLine(0, 16, SCREEN_WIDTH, 16, SSD1306_WHITE);

    // Bottom 48 pixels (often Blue)
    if (millis() - lastValueReceived > 30000 || lastValueReceived == 0) {
      display->setTextSize(2); // Medium font for error
      display->setCursor(0, 24);
      display->print("NO DATA");
    } else {
      display->setTextSize(3); // Big text for value
      display->setCursor(0, 24);
      display->print((int)gridPower);
      display->print("W");
    }
  } else {
    // ---- SCREEN 2: Placeholder ----
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->println("System Info");

    display->drawLine(0, 16, SCREEN_WIDTH, 16, SSD1306_WHITE);

    display->setTextSize(2);
    display->setCursor(0, 24);
    display->print("Screen 2...");
  }

  // Bottom edge Uptime
  display->setTextSize(1);
  display->setCursor(0, SCREEN_HEIGHT - 8);
  display->print("Uptime: ");
  display->print(c);
  display->print("s");

  display->display();
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  mqttClient.setServer(SECRET_MQTT_SERVER, 1883);
  mqttClient.setCallback(mqttCallback);

  Wire.begin(OLED_SDA, OLED_SCL);  // Wire.begin must be called explicitly with custom i2c pins on ESP8266 or supply pins via constructor
  display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

  if(!display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  c++;
  delay(1000);
}

#endif
