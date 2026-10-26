# Decisions

Running log, newest at the bottom. If I change my mind I add a new entry instead of rewriting the old one. "Untested" means it's a bet I still have to check. Dates are month-level where I didn't write the day down at the time.

## D-001: Rover, not an arm (Oct 2026)

I wanted something I could actually finish as a first robot. An arm has to carry every downstream joint's weight, so torque stacks up at the shoulder and an SG90 runs out almost immediately. A bit of slop at each joint adds up at the gripper and you can't tell which joint caused it. It also wants inverse kinematics before anything moves. A rover gets its support from the floor, and the interesting part, closed-loop speed control, is software I can add once the thing rolls. The arm is a possible second project.

## D-002: Two driven wheels and a caster, not 4WD (Oct 2026)

4WD needs four motor channels, or skid steer and the traction fights that come with it. Differential drive teaches the same control and kinematics with half the failure points. I only lose traction on rough ground and this is an indoor robot.

## D-003: ESP32 and encoder motors (Oct 2026)

ESP32 over Arduino Uno for the WiFi (phone control, maybe telemetry later), the extra interrupt-capable pins and the PWM channels. The cost is 3.3 V logic, so the HC-SR04 echo needs a divider.

N20 motors with built-in encoders, about $4 more each. Without them there's no feedback, and the closed-loop control is the part I care about. Plain N20s would just be open-loop PWM.

## D-004: Motor bracket is a friction-fit pocket, not flange bolts (Oct 2026)

N20 flange hole spacing varies between suppliers and I didn't have a motor in hand. The pocket only depends on the can cross-section (12 x 10 mm, modelled 12.3 x 10.3 for 0.15 mm a side), and the gearbox flange is wider than the pocket so it stops on the shoulder. That shoulder face is the datum that locates the motor and the wheel.

Two tabs with 3.4 mm holes, 6 mm long, 8 mm wide, 3 mm thick, 2 mm in from each end, filleted at the body. I first made them 10 mm long, which overlap on a 20 mm block. Holes cut with Up To Next so they stop at the tab and don't reach the pocket. Printed on its back face, tabs down, no supports.

Untested: pocket depth is 15.3 mm and I don't know how much of the 26 mm motor is can. One printed, never fitted to a motor.

## D-005: Chassis plate layout (Oct 2026)

150 x 110 x 4 mm with 5 mm corner radii, centred on the origin so mirrored features stay simple. Four 4.2 mm holes at X = +-40 and +-50 for M3 heat-set inserts, with 6 mm bosses 2 mm tall underneath because 4 mm of plate is thin for an insert.

Battery pocket is 44 x 78 mm and 1.5 mm deep, with a 12 x 4 mm wire notch out the front edge. My first version ran the long side along X and landed on the motor holes. I'd mixed up which axis was the 150 mm one, so the long side runs along Z now.

## D-006: Slotted caster holes (Oct 2026)

12 mm slots at X = +-15, 3.4 mm wide. I haven't bought the caster and hole spacing varies a lot between small ones, so the slots absorb whatever it turns out to be.

## D-007: Sensor wall sits on the plate (Oct 2026)

The first version was a wall extruded forward off the front edge. It only touched the plate along a line, so it was a weak joint and would have needed supports. Now it's a 50 x 6 x 22 mm block on the plate with its front face flush with the plate edge, so it has real contact area and the sensor still sees forward.

Update: the real board has four corner holes, about 42 x 17.5 mm and 2 mm (ruler only), not the two I modelled. The right-angle header on the back points down into the plate when mounted the obvious way, so the plan is to mount the board upside down, put four 1.8 mm blind holes for M2 screws in the wall and cut a notch at the top for the header. Not modelled yet. Test-print just the wall first.

## D-008: Two buck converters (Oct 2026)

The motors are 6 V and the pack is 6.0 to 8.4 V, so running them off the pack would overdrive them by up to 40%. One buck set to 6 V feeds the driver, one set to 5 V feeds the ESP32 and the sensor. The alternative was one buck and a PWM cap in firmware, which works until the battery sags or I forget the cap, and it eats the PID's headroom.

The holder I bought is bare, no switch or protection, so protected cells and an inline switch go on the list. A 100 to 470 uF cap goes across the motor rail so startup spikes don't reset the ESP32.

## D-009: One chassis reprint, not one per fix (Oct 2026)

The sensor wall is part of the chassis, so any change means a whole reprint. Rather than reprint for each fix, I'm saving them: sensor wall, the ride height, and the ESP32 and driver mounts, which can't be designed until I can measure the boards.

Ride height is the big one and it's unresolved. The motor axis is about 16 mm above the plate underside, the wheel radius is 21 mm, and I'd assumed a caster riser would fix the difference. A riser can only make the caster taller, and the gap needs it shorter. Options are recessing the motor brackets into the plate, a low-profile ball caster, or a skid. I'll decide with the real caster in my hand.

## D-010: PID gains from a placeholder model (Oct 2026)

The firmware's first gains were 2.0, 0.5 and 0.1, picked without a model. In sim/ the speed loop, with guessed motor numbers, chatters between full forward and full reverse with those, flipping every 50 ms. With the motor settling faster than the 50 ms sample, it behaves like a plain gain K of about 6.8 ticks/s per PWM count, and the loop is stable only if K * Kp < 1 and 2 * K * Kp + K * Ki * Ts < 2. With Ki = 1 that puts Kp under about 0.12. The pole math says Kp = 0.05 with Ki = 1 settles in about 0.75 s. I haven't run that case in the sim yet.

Those numbers belong to the guessed motor. The firmware still has the old gains. I'll fit the model to a step test on the real motor before touching them, and Kd stays at zero until P and I behave, since quantisation turns one tick of noise into a big derivative spike.

## D-011: Phone control is a web page on the ESP32 (Oct 2026)

The ESP32 joins my home WiFi and serves one page with five buttons. Each button hits an endpoint that sets the two target wheel speeds, and the PID loop reads them on its next 50 ms update, so the web code never touches the motors. Opposite signs on the wheels turn it in place. Station mode, so it only works inside WiFi range.

No failsafe yet, see R4.

## D-012: Bench sketches before the real firmware (Oct 2026)

One sketch per subsystem (board, motors, encoders, ultrasonic, WiFi), run in order, so when something fails I know which part it is. Wheels off the ground for the motor test.

Update 2026-10-25: the ESP32 wouldn't take an upload. Device Manager showed nothing new when I plugged it in, only COM1 in the IDE. The LED lights, so it has power. Charge-only cable is the main suspect, and a different cable is the first thing to try.

## D-013: Repo layout follows myo-hand (2026-10-25)

Requirements and decisions live in docs/, CAD in cad/, firmware in firmware/, sim in sim/. Simulink's cache folder isn't tracked. The firmware had a lot of tutorial comments in it from when I was learning it, and I cut those down to the ones that say why.
