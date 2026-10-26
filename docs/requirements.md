# Requirements

What the rover has to do, how I'll check each requirement, and the numbers behind them. Most numbers come from product listings or guesses. The assumptions table at the bottom says which.

## The problem

I want a robot I designed, printed, wired, programmed and tuned myself, small and cheap enough that a failed attempt isn't a disaster. It should drive straight, take commands from my phone, and not drive into a wall. I picked a rover over an arm because ground contact takes the torque-stacking and tolerance-stacking problems off the table (D-001).

## Scope

In: two driven wheels and a caster. WiFi control from a phone browser. Encoder feedback with a PID loop per wheel. An ultrasonic stop in front. A printed chassis.

Out: line following, SLAM, lidar, a camera, IMU heading hold, 4WD, ROS 2, outdoor use, and anything that tries to map the room. Some of that is a second version if this one drives.

## Constraints

- Every printed part fits in the A1 Mini's 180 x 180 x 180 mm bed.
- The first parts list came to about $110 to 120 CAD including calipers and a soldering iron. The second buck converter, multimeter, protected cells, switch, resistors, capacitor and M2 screws came after that. There's no hard cap but it shouldn't keep creeping.
- Battery power only, two 18650 cells (6.0 to 8.4 V).
- The motors are 6 V parts, so they can't run straight off the pack.
- The ESP32 is 3.3 V logic. The HC-SR04 is a 5 V part.

## Requirements

Must means V1 fails without it. Should can be dropped with a reason. Could is nice to have.

| ID | Requirement | Where it comes from | Priority | How I'll check |
|----|-------------|---------------------|----------|----------------|
| R1 | Drive and stop from a phone browser on the same 2.4 GHz network | Project goal | Must | All five buttons work, from a phone |
| R2 | Hold a straight line | Project goal, the reason for the encoders | Must | Drive 2 m on a smooth floor with PID off, then on, measure sideways drift |
| R3 | Stop before touching a flat obstacle approached at drive speed | Project goal | Must | 10 runs into a box, count touches |
| R4 | Stop if the phone connection drops | Safety, and the firmware currently doesn't | Should | Kill the phone's WiFi mid-drive |
| R5 | Every printed part fits in the A1 Mini bed | Constraint | Must | Slicer preview |
| R6 | Both drive wheels and the caster on the floor, plate level | Ride height problem (D-009) | Must | Ruler across the plate, or a level |
| R7 | Swap the battery without taking anything apart | Why the pocket has an open notch | Could | Do it |

R2 and R3 don't have numeric pass marks yet. I'll measure the open-loop drift and the stopping distance first, then set targets that closed-loop and the sensor have to clearly beat. Picking a number now would be made up.

## The numbers

From the listings: motor 12 x 10 x 26 mm, 3 mm D-shaft, 150 RPM at 6 V no load. Wheel 42 x 19 mm. Battery holder 74.5 x 40.8 x 17.7 mm. HC-SR04 about 45 x 20 x 15 mm. Plate is 150 x 110 x 4 mm.

No-load top speed: 150 RPM x 132 mm wheel circumference is about 0.33 m/s. Real speed under load will be lower.

Motor axis height: the bracket puts the pocket centre 6.85 + 5.15 = 12 mm above the bracket tab faces, plus the 4 mm plate, so about 16 mm above the plate underside. Wheel radius is 21 mm, so the wheel bottom is about 5 mm below the plate. The caster I bought is 34.5 mm tall, so the brackets go under the plate instead (D-014): axis 17 mm below the plate underside, plate 38 mm off the floor, so the caster needs a 3.5 mm spacer.

Speed resolution: the firmware counts one tick per encoder pulse, and updates every 50 ms, so the smallest step it can see is 1 / 0.05 = 20 ticks/s.

Power: the pack is 6.0 to 8.4 V and the motors are 6 V, so straight off the pack they'd run about 40% over at full charge. Hence one buck at 6 V for the driver and one at 5 V for the ESP32 and sensor. Echo divider: 5 V x 2k / (1k + 2k) = 3.3 V.

## Assumptions

None of these are verified yet.

| ID | Assumption | How I'll check |
|----|------------|----------------|
| A1 | Motor can fits the 15.3 mm bracket pocket and the flange seats on the shoulder | Calipers on the real motor |
| A2 | Encoder is 7 pulses per rev at the motor shaft, and the gearbox is 100:1 (sim/motor_params.m uses 700 ticks per output rev) | Turn a wheel one revolution in bench test 3 and read the count |
| A3 | Motor resistance, inertia and friction in motor_params.m (all guesses) | Step test on the real motor, fit the model |
| A4 | Caster height | Measure it when it arrives |
| A5 | HC-SR04 hole pattern is 42 x 17.5 mm and about 2 mm, from a ruler | Calipers, then a test print of the wall |
| A6 | 0.15 mm per side is a good slide fit for the motor pocket on this printer | One bracket is printed, never tried with a motor |
| A7 | Encoders run fine on 3.3 V | Datasheet or bench test 3 |
