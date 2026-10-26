% rover_motor_params.m
% Placeholder N20 motor + wheel parameters -> first-order model for Simulink.
% Everything marked (GUESS) must be replaced with measured values once the
% real motors arrive. Run this BEFORE opening the Simulink model, so the
% variables (K_ticks, tau, Ts) exist in the base workspace.

% --- From the product listing ---
V_rail = 6;                  % V, motor buck output
rpm_nl = 150;                % rpm no-load at 6 V (listing)
w_nl   = rpm_nl*2*pi/60;     % rad/s at the output shaft

% --- Guesses ---
R  = 7.5;     % ohm      (GUESS: assumes roughly 0.8 A stall current)
J  = 2e-4;    % kg*m^2   (GUESS: gearbox-reflected rotor + wheel + share of chassis)
b  = 1e-4;    % N*m*s    (GUESS: viscous friction)
ticks_per_rev = 700;         % (GUESS: 7 pulses/rev x 100:1 gearing. Check the
                             %  listing. The firmware counts rising edges of A only.)

% --- Derived ---
Ke = V_rail / w_nl;          % V*s/rad, back-EMF constant
Kt = Ke;                     % N*m/A (equal in SI units)

% Electrical inductance is tiny for an N20, so neglect it. The motor
% becomes first-order: K_dc / (tau*s + 1)
K_dc = Kt / (R*b + Kt*Ke);   % rad/s per volt
tau  = R*J / (R*b + Kt*Ke);  % seconds

% --- Convert to the firmware's units ---
% Input: PWM counts (-255..255) mapped to 0..V_rail. Output: encoder ticks/s.
Ts          = 0.05;                       % s, PID period in the firmware
V_per_count = V_rail / 255;
K_ticks     = K_dc * V_per_count * ticks_per_rev / (2*pi);  % (ticks/s) per PWM count

fprintf('K_dc    = %.2f rad/s per volt\n', K_dc);
fprintf('tau     = %.1f ms   (PID period is %.0f ms)\n', tau*1e3, Ts*1e3);
fprintf('K_ticks = %.2f ticks/s per PWM count\n', K_ticks);
