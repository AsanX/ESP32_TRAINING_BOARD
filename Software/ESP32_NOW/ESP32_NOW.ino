#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Temperature_LM75_Derived.h>

#define SCREEN_WIDTH 128  // OLED width in pixels
#define SCREEN_HEIGHT 64  // OLED height in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET    -1  // Reset pin not used
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Set up the rgb led names
uint8_t ledR = 18;
uint8_t ledG = 17;
uint8_t ledB = 16;

#define CHARGING_PIN 13     // Pin 13 (Charging signal from TP4056)
#define STANDBY_PIN 23      // Pin 23 (STANDBY signal from TP4056)
#define EN_PIN 4            // Pin 4 (EN signal for TP4056)

const boolean invert = true;  // set true if common anode, false if common cathode

uint8_t color = 0;         // a value from 0 to 255 representing the hue
uint32_t R, G, B;          // the Red Green and Blue color components
uint8_t brightness = 255;  // 255 is maximum brightness, but can be changed.  Might need 256 for common anode to fully turn off.

Generic_LM75_12Bit temperature;

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

  ledcAttach(ledR, 12000, 8);  // 12 kHz PWM, 8-bit resolution
  ledcAttach(ledG, 12000, 8);
  ledcAttach(ledB, 12000, 8);

  // Set pins 13 and 23 as input
  pinMode(CHARGING_PIN, INPUT_PULLUP);   
  pinMode(STANDBY_PIN, INPUT_PULLUP);  
  pinMode(EN_PIN, INPUT_PULLUP);    

  analogReadResolution(12);
}

// void loop runs over and over again
void loop() 
{
    int chargingState = digitalRead(CHARGING_PIN);   // 0 or 1 for Charging signal
    int standbyState = digitalRead(STANDBY_PIN);     // 0 or 1 for Standby signal

    chargingState ? ledcWrite(ledB, 0) : ledcWrite(ledB, 200);
    standbyState ? ledcWrite(ledG, 0) : ledcWrite(ledG, 200);

      // read the analog / millivolts value for pin 2:
    int analogValue = analogRead(36);
    int analogVolts = analogReadMilliVolts(36);
    float tempC = temperature.readTemperatureC();  // Read temperature in Celsius

    // === OLED output ===
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(F("Battery: "));
    display.print(convertadctobat((uint16_t)analogVolts));
    display.println(F(" mV"));
    display.setCursor(0, 20);
    display.setTextSize(1);
    display.print(F("Temp: "));
    display.print(tempC);
    display.println(F(" C"));
    display.display();

    delay(10);

}

uint16_t convertadctobat(uint16_t mV)
{
  // Reverse voltage divider to get actual battery voltage (also in millivolts)
  // Vbat = Vadc * (R1 + R2) / R2
  uint16_t vBat = (uint32_t)mV * (33000 + 8200) / 8200;

  return vBat;
}