# ESP32_TRAINING_BOARD

ESP32 Training board is designed to have multiple things to play with so there are lot's of opportunities to learn the ESP32.
I made this board so i can remotely open our front door from 4th floor and figured i could do something else with it too.

- USB-C Charging, programming and serial monitor
- 18650 Battery with commonly used TP4056 charging circuit
- RGB Led
- Button / Encoder
- TMP100 12Bit I2C Temperature sensor
- OLED I2C Display
- Few optocoupler for inputs and outputs

# Alredy found mistakes from HW 1.0
## Enable pin and auto reset
When using automatic reset for programming, there needs to be a uF range capacitor on enable.
This is to make sure the IO0 pin is given some time to pull low to enter the download mode before the en pin gets HIGH.
![Alt text for image](Hardware/BUTTON_OPENER/EN%20mistake.png)

## TP4056 temperature pin
Datasheet clearly says that the pin should be pulled low if there are no temperature sensor on the battery
![Alt_text_for_image](Hardware/BUTTON_OPENER/TP4056%20mistake.png)
