# Wiring

Plan only, nothing is wired yet. The block diagram is docs/wiring.png.

## Power

Battery (2x18650, 6.0 to 8.4 V) goes through a switch into two LM2596 bucks.

- Buck 1, set to 6.0 V: driver motor supply (VM), which feeds both N20s.
- Buck 2, set to 5.0 V: ESP32 VIN and the HC-SR04.
- ESP32 3V3 pin: driver logic supply (VCC) and the encoders.
- One shared ground across everything.
- 100 to 470 uF electrolytic across VM, close to the driver.

The LM2596 modules come with an arbitrary output voltage. Power each one from the battery with nothing on the output, set it with a multimeter, and label it before anything gets connected. Don't power the ESP32 from USB and the 5 V buck at the same time until I've checked how this board isolates the two.

## Pins

| Signal | GPIO | Goes to |
|--------|------|---------|
| STBY | 27 | TB6612FNG STBY |
| Left IN1 / IN2 / PWM | 26 / 25 / 14 | AIN1 / AIN2 / PWMA |
| Right IN1 / IN2 / PWM | 33 / 32 / 13 | BIN1 / BIN2 / PWMB |
| Left encoder A / B | 34 / 35 | left motor encoder |
| Right encoder A / B | 36 / 39 | right motor encoder |
| TRIG | 5 | HC-SR04 Trig |
| ECHO | 18 | HC-SR04 Echo, through the divider |

Check these against the pinout of the actual board when I have it. The firmware #defines are the source of truth.

The HC-SR04 echo is 5 V and the ESP32 pins are 3.3 V. 1k in series from Echo to GPIO18, 2k from GPIO18 to ground. Two 1k in series works for the 2k.
