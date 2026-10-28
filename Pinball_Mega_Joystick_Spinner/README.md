# Pinball MEGA — joystick pop bumper + spinner

This is an updated version of your **Pinball_Mega_RGB_StarWars** sketch. Open `Pinball_Mega_Joystick_Spinner.ino` in Arduino IDE; keep **all** the `.cpp` / `.h` files in this same folder. Choose **Arduino Mega or Mega 2560** and the correct port.

## New wiring

- **Analog joystick module:** VCC -> Mega 5V, GND -> Mega GND, VRx -> Mega A0, VRy -> Mega A1; joystick SW not used. Let the stick rest at center while the controller powers up (automatic center calibration).
- **Spinner's COM/NO pair:** the **left and middle** terminals you confirmed only beep when pressed. Connect one to Mega **GND** and the other to **D2** (either order). Leave the third terminal disconnected. Do not connect voltage to the spinner switch.
- **Bumper MOSFET/driver control:** Mega **D37** is unchanged. This is *driver logic only*, never a direct connection to a solenoid or a 24V supply. **Pin D34 is no longer used** for the bumper sensor.
- Your scoring IR beams, loss sensor, two servo pins, LCD pins, RGB LEDs, buzzer, and Uno-game-enable wiring have not changed.

## Behavior

- **Joystick:** a change in *either axis* of approximately 170 ADC counts from the calibrated center, sustained 5 ms, registers a bumper hit. Adds 500 points, triggers bumper sound/LED effect, and requests a bumper driver pulse (only when separately armed). Must settle within 90 counts of center for 30 ms before a new event. A 500 ms cooldown is also enforced by `PopBumper`.
- **Spinner:** D2 external interrupt counts switch closures (FALLING edge), suppressing edges closer than 3000 microseconds to reduce contact bounce. Adds 100 points per accepted edge, using your existing score/LCD/sound/LED logic. Scores only when a game is running.
- **Other game rules preserved:** auto-start, 15-second ball release cycle, five loss events end the game, best score in EEPROM, Star Wars background, RGB animations, 3 scoring sensors, start/restart and serial tests.

## Test commands (Serial Monitor at 9600 baud)

- `S` = simulate one spinner pulse (+100)
- `B` = simulate a joystick/bumper hit (+500, subject to bumper cooldown)
- `1`, `2`, `3` = simulate scoring beams
- `L` = simulate ball loss; five losses end the game
- `N` = start new game (after game over)
- `X` = finish game; `T` = cycle LED tests

**Safe testing:** `Settings::ARM_BUMPER_OUTPUT = false` in `Config.h`. Keep it false while testing scoring and joystick detection, with the 24V coil power DISCONNECTED. The D37 activation duration of 20 ms is ONLY a placeholder and must not be assumed safe. Before operating a real coil, have a qualified supervisor verify coil ratings/duty cycle, correctly rated MOSFET driver, flyback suppression, wiring, fusing, physical guards and independent power shutdown. The Arduino must not directly supply the coil. Test movements without a steel pinball until the bumper mechanism is guarded.

## Calibration/tuning

- Change `JOYSTICK_TRIGGER_DELTA` in `Config.h` (higher = less sensitive). `JOYSTICK_RELEASE_DELTA` should be smaller than trigger delta. If impacts are too brief, tune `JOYSTICK_HIT_STABLE_MS`, but beware false triggers from vibration.
- `SPINNER_POINTS` and `BUMPER_POINTS` control scoring.
- If spinner counts double or misses fast spins, tune `SPINNER_DEBOUNCE_US` and inspect waveforms. Mechanical contacts bounce; a fast spinner plus NeoPixel timing can still miss pulses. For guaranteed counting at extreme speed, use dedicated hardware pulse conditioning/counter.
- Never move the joystick during the startup calibration. The joystick module is delicate and is not intended for direct impacts by a steel pinball: use an isolated, mechanically protected actuator or a real bumper switch.
- This ZIP uses the same NeoPixel library dependency as your RGB StarWars version. For default 3-wire addressable strips (`PINBALL_LED_STRIP_TYPE == 1`), install `Adafruit NeoPixel` through Arduino IDE Library Manager.

## Remaining pin map

| Device | Pin(s) |
|---|---|
| Buzzer | D8 |
| Release/tunnel servos | D9 / D10 |
| Joystick | A0 / A1 |
| Spinner | D2 |
| Score 1/2/3 | D30 / D31 / D32 |
| Loss | D33 |
| Optional start | D35 |
| Uno game enable | D36 |
| Bumper MOSFET driver logic | D37 (OFF by default) |
| RGB NeoPixel data | D40 (if configured as addressable) |
| 4-wire RGB driver inputs | D44 / D45 / D46 (if configured as analog strips) |
| LCD RS/E/D4-D7 | D22 / D23 / D24-D27 |

No changes to the Arduino Uno flippers/continuous elevator sketch are required for these two scoring inputs.
