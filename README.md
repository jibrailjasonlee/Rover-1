# rover-1

A two-wheel rover on an ESP32. Each wheel has an N20 gearmotor with an encoder and its own PID loop, and you drive it from a phone browser over WiFi. An HC-SR04 on the front is supposed to stop it before it hits things. Everything printed fits on a Bambu A1 Mini.

It's my first robot. The goal is doing the whole chain once, CAD to wiring to firmware to tuning, and finding out where it breaks.

It doesn't drive yet.

## Status

The chassis plate and one motor bracket are modelled and printed. The firmware compiles and has never run on a board. The ESP32 is here but won't take an upload, most likely a charge-only USB cable. There's a Simulink model of the speed loop, built on guessed motor numbers.

The chassis gets reprinted before anything is mounted on it: sensor wall, ride height and the board mounts all change. See docs/decisions.md.

## What's here

    cad/                       SolidWorks parts, with STEP/STL/3MF next to them where I've exported one
    firmware/rover/            the main sketch
    firmware/bench/            one sketch per subsystem, run in order 01 to 05
    sim/                       Simulink speed loop, motor_params.m sets the numbers it uses
    docs/requirements.md       what it has to do and how each requirement gets checked
    docs/decisions.md          design decisions, newest at the bottom
    docs/wiring.md             power layout and pin map
    docs/wiring.png            same thing as a block diagram
    docs/rover_explainer.html  click-through explanation of the firmware, opens in a browser
    docs/rover_overview.html   the finished rover drawn to scale, every part, its job and why, opens in a browser

You need SolidWorks for the .SLDPRT files and MATLAB with Simulink for the .slx. The rest opens anywhere.

## Running it

Arduino IDE with the Espressif esp32 board package. Pick "DOIT ESP32 DEVKIT V1", or "ESP32 Dev Module" if the pins act wrong. Each sketch has to sit in a folder with its own name, which they do.

Put your WiFi name and password in rover.ino before uploading and don't commit them. It's 2.4 GHz only, and eduroam won't work. The serial monitor (115200) prints the address to open on your phone.

Run the bench sketches in order before the main one, with the wheels off the ground. Set both buck converters with a multimeter before the ESP32 goes anywhere near them.

## Known problems

Nothing is wired. Every number in the firmware and the sim is a guess until the motors are on a bench.

The PID gains in rover.ino (2.0 / 0.5 / 0.1) are placeholders and the sim says they make the loop chatter between full forward and full reverse. Don't tune from them. Speed is in encoder ticks per second, not RPM, and with one tick per pulse a 50 ms window can only resolve steps of 20 ticks/s.

Firmware gaps: readDistance() is an empty stub, so no obstacle stop. Nothing zeroes the targets if the phone drops. The integral has no anti-windup, and the first PID call after boot sees a huge dt.

Mechanical gaps: the v2 chassis is in CAD and unprinted. Brackets hang under the plate (D-014) so the axle is 17 mm below it and the plate sits 38 mm off the floor, which the assembly confirms. Wheel notches and four caster pads (3.5 mm, the spacer) are modelled. The sensor wall is still being redone for the four-hole board. The ESP32, driver and buck mounts don't exist. The bracket pocket is 15.3 mm deep and I haven't measured how much of the 26 mm motor goes into it.

The battery holder is a bare one with no switch or protection, so protected cells and an inline switch are on the list.
