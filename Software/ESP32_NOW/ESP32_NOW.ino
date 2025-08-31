#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ESP32_NOW.h"
#include "WiFi.h"

#define SCREEN_WIDTH 128  // OLED width in pixels
#define SCREEN_HEIGHT 64  // OLED height in pixels

#define ESPNOW_WIFI_CHANNEL 6

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET    -1  // Reset pin not used
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Set up the rgb led names
uint8_t ledR = 18;
uint8_t ledG = 17;
uint8_t ledB = 16;

#define CHARGING_PIN 13     // Pin 13 (Charging signal from TP4056)
#define STANDBY_PIN 23      // Pin 23 (STANDBY signal from TP4056)
#define EN_PIN 4            // Pin 4  (EN signal for TP4056)
#define SW_PIN 32           // Pin 32 (SW / Encoder SW)

struct Input {
  const uint8_t PIN;
  uint32_t events;
  bool active;
};

Input charging = {CHARGING_PIN, 0, false};
Input standby = {STANDBY_PIN, 0, false};
Input sw = {SW_PIN, 0, false};


/* Interrupt service*/
void ARDUINO_ISR_ATTR isr(void *arg) 
{
  Input *s = static_cast<Input *>(arg);
  s->events += 1;
  s->active = true;
}

class ESP_NOW_Broadcast_Peer : public ESP_NOW_Peer {
public:
  // Constructor of the class using the broadcast address
  ESP_NOW_Broadcast_Peer(uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(ESP_NOW.BROADCAST_ADDR, channel, iface, lmk) {}

  // Destructor of the class
  ~ESP_NOW_Broadcast_Peer() {
    remove();
  }

  // Function to properly initialize the ESP-NOW and register the broadcast peer
  bool begin() {
    if (!ESP_NOW.begin() || !add()) {
      log_e("Failed to initialize ESP-NOW or register the broadcast peer");
      return false;
    }
    return true;
  }

  // Function to send a message to all devices within the network
  bool send_message(const uint8_t *data, size_t len) {
    if (!send(data, len)) {
      log_e("Failed to broadcast message");
      return false;
    }
    return true;
  }
};

/* Global Variables */
uint32_t msg_count = 0;

// Create a broadcast peer object
ESP_NOW_Broadcast_Peer broadcast_peer(ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, nullptr);

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);
  delay(10);

   // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    while(true);  // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner
  display.display();                  // Show on screen

  ledcAttach(ledR, 3000, 8);  // 12 kHz PWM, 8-bit resolution
  ledcAttach(ledG, 3000, 8);
  ledcAttach(ledB, 3000, 8);

  // Set pinmodes
  pinMode(CHARGING_PIN, INPUT_PULLUP);   
  pinMode(STANDBY_PIN, INPUT_PULLUP);  
  pinMode(SW_PIN, INPUT_PULLUP);  
  pinMode(EN_PIN, OUTPUT_OPEN_DRAIN); 

  attachInterruptArg(SW_PIN, isr, &sw, FALLING);

  analogReadResolution(12);

  // Initialize the Wi-Fi module
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  while (!WiFi.STA.started()) {
    delay(100);
  }

  Serial.println("ESP-NOW Example - Broadcast Master");
  Serial.println("Wi-Fi parameters:");
  Serial.println("  Mode: STA");
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

  // Register the broadcast peer
  if (!broadcast_peer.begin()) {
    Serial.println("Failed to initialize broadcast peer");
    Serial.println("Reebooting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  Serial.printf("ESP-NOW version: %d, max data length: %d\n", ESP_NOW.getVersion(), ESP_NOW.getMaxDataLen());

  Serial.println("Setup complete. Broadcasting messages every 5 seconds.");

}


// void loop runs over and over again
void loop() 
{
  if (sw.active == true)
  {
    ledcWrite(ledB, 255);
    sw.active = false;

      // Broadcast a message to all devices within the network
    char data[32];
    snprintf(data, sizeof(data), "Hello, World! #%lu", msg_count++);

    Serial.printf("Broadcasting message: %s\n", data);

    if (!broadcast_peer.send_message((uint8_t *)data, sizeof(data))) {
      Serial.println("Failed to broadcast message");
    }

  }

  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 3000) {
    lastMillis = millis();
    ledcWrite(ledB,0);
    ledcWrite(ledR,0);
    ledcWrite(ledG,0);
  }
}

/* Use this to convert ADC millivolts to Battery millivolts */
uint16_t adcmV_to_batmV(uint16_t mV)
{
  // Reverse voltage divider to get actual battery voltage (also in millivolts)
  // Vbat = Vadc * (R1 + R2) / R2
  uint16_t vBat = (uint32_t)mV * (33000 + 8200) / 8200;

  return vBat;
}