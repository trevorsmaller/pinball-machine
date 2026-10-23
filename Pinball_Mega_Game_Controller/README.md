# Pinball Machine — Arduino Mega 2560 Game Controller

This is **Arduino #2**. Arduino Uno #1 runs the **two flippers and continuously running worm-elevator stepper** on its own program. The Mega handles the rest: five-ball game state, periodic ball release, losing gate, three scoring sensors, pop-bumper scoring/driver output, LCD, LED event effects, background music + sound events, EEPROM high score, and a game-enable output to the Uno.

## Files

- `Pinball_Mega_Game_Controller.ino`: top-level `setup()`, `loop()`, start/game-over, scoring, test commands
- `Config.h`: every pin, scoring value, servo angle, timing and safety setting
- `SensorInputs.*`: revised team IR/microswitch sensors with stable-state debounce and one event per activation
- `ServoGates.*`: adapted team's `ActualServo.cpp` without blocking, integrated to game state
- `SoundController.*`: adapted five event-tone arrays from `buzzerbackground.cpp`; responsive background playback
- `HighScoreStore.*`: Arduino EEPROM high score, saved at end of game if a new record
- `LCDView.*`: adapted parallel 16x2 `LiquidCrystal` interface and dynamic score/high display
- `LightEffects.*`: new nonblocking LED event effects
- `PopBumper.*`: new cooldown and timed *driver control output*, disabled by default until electrically validated

All `.ino`, `.cpp`, `.h` files **must be in one folder** named `Pinball_Mega_Game_Controller`, then open that `.ino` in Arduino IDE and select **Arduino Mega or Mega 2560**.

Libraries used are built into standard Arduino AVR installation: `Servo`, `LiquidCrystal`, and `EEPROM`. No `pitches.h` required. The team's original background melody referenced that missing header, so a short original background riff is supplied; the five team event sound effects use the same note frequencies/times.

## Hardware assumptions and pin map

| Component | Mega pin | Input/output |
|---|---:|---|
| Buzzer (passive piezo) | 8 | `tone()` output |
| Ball release servo | 9 | servo output |
| Tunnel-stop servo | 10 | servo output |
| LCD RS | 22 | output |
| LCD E | 23 | output |
| LCD D4 | 24 | output |
| LCD D5 | 25 | output |
| LCD D6 | 26 | output |
| LCD D7 | 27 | output |
| Scoring IR beam 1 | 30 | active LOW |
| Scoring IR beam 2 | 31 | active LOW |
| Scoring IR beam 3 | 32 | active LOW |
| Losing-hole IR beam | 33 | active LOW |
| Pop-bumper hit microswitch | 34 | active LOW |
| Start/restart button | 35 | active LOW |
| Game-enable signal to Uno D7 | 36 | LOW=game running; released/high-impedance=not running |
| Pop bumper MOSFET/driver logic | 37 | active HIGH pulse; DISABLED by default |
| Scoring-effect LED | 40 | output |
| Lost-ball-effect LED | 41 | output |
| Status-effect LED | 42 | output |

**Sensors and button:** For the assumed active-low outputs, connect each digital sensor signal to its listed pin. For simple buttons/switches, use switch between pin and Mega GND. `INPUT_PULLUP` is enabled on all inputs. If an IR sensor outputs HIGH on detection, invert its logic in `SensorInputs.cpp` before using it. Verify sensor voltage is compatible with 5V Mega digital inputs.

**LCD:** 16x2 HD44780-style parallel LCD; use RS, E, D4-D7 as above, R/W to GND, VSS/GND, VDD/5V and the correct contrast/backlight circuit for your LCD. The older `LCD_Screenlogic.ino` used conflicting Uno pin numbers and is intentionally *not* included.

**LEDs:** Use appropriately sized current-limiting resistors and consider LED drivers for high-power lamps; Arduino pins must not directly drive larger loads.

**Servos:** The angles `RELEASE_OPEN_ANGLE`, `RELEASE_CLOSED_ANGLE`, `TUNNEL_OPEN_ANGLE`, and `TUNNEL_CLOSED_ANGLE` in `Config.h` are only initial assumptions. Use a suitable external regulated servo supply and share logic GND with the Mega; do not power multiple loaded servos from the Mega 5V pin.

**Pop bumper (24V solenoid):** Arduino D37 is ONLY a low-voltage control signal to an externally rated driver, **never** a connection to a solenoid coil or a 24V circuit. Electrical design must include an appropriate external supply, rated driver, flyback/transient protection, fusing, and a fail-safe independent power cutoff; confirm duty-cycle, driver and coil limits. `ARM_BUMPER_OUTPUT = false` is the default, so you can validate events/scores while the pop solenoid stays off. `BUMPER_PULSE_MS = 20` is a placeholder, **not** a validated safe energization time. Do not set `ARM_BUMPER_OUTPUT = true` before validating those requirements. A stalled/repeated coil can overheat even with a software timeout.

**Mega -> Uno:** The earlier Uno sketch uses Uno `GAME_ENABLE_PIN = 7`, **active LOW**, `INPUT_PULLUP`. Connect Mega D36 to Uno D7 and **share logic GND**. Change the Uno setting `USE_GAME_ENABLE_INPUT = true` (it was previously `false`). The Mega emulates an open-drain output: when running it drives LOW, otherwise it switches to INPUT/high impedance, allowing the Uno pull-up to read HIGH. Add appropriate protection/isolation when using independent power systems or long wires; avoid cross-powering boards. This signal is gameplay coordination, NOT an electrical emergency-stop circuit. Servo game-over operates locally on the Mega.

## Game rules implemented

1. Power-up: no game running, both servos in CLOSED positions, Uno disabled; LCD shows `PRESS START` and saved high score.
2. Press button on Mega D35 to start. Score = 0, lost balls = 0, tunnel opens, Uno enabled. Game-start sound/LED effect.
3. Beginning **15 seconds after start**, the release servo opens briefly, then closes. Repeat every 15 seconds (Option B), whether or not a ball has been lost; no ball inventory sensor is provided.
4. Three scoring hole beams add the configured points (defaults: 100/200/300). The ball returns mechanically via the scoring-hole track; code does not operate a scoring-hole return gate.
5. Pop-bumper sensor triggers a 500-point score event (and an optional controlled driver pulse, currently disabled).
6. Each distinct losing-hole detection increments the loss counter; after the fifth event, both servos close, Uno is disabled, score is saved if a new high, and game-over effects play.
7. The LCD displays `Score:` and `High:` during gameplay. `High:` reflects any new score instantly; EEPROM writes only on game completion. Press start after game over for a new game. Press and hold start for three seconds while playing to end the game manually.

`millis()` replaces `delay()` for all gameplay, servo, sound and LED logic. Sensors must return to the idle/unblocked state before they can create another event. There is also a configurable minimum time between events.

## Test without coils

1. Upload the sketch with `ARM_BUMPER_OUTPUT = false` and safe test loads. Don't power solenoids through the Arduino. Verify control signals with LEDs, a meter or approved test equipment.
2. Open Serial Monitor at 9600 baud. Commands:
   - `N`: start a new game
   - `1`, `2`, `3`: simulate the respective scoring hole
   - `B`: simulate a pop-bumper hit / add points
   - `L`: simulate one distinct lost ball
   - `X`: end the game early
3. Example test: send `N`, `1`, `2`, `3`, `B`, then five `L` characters. Expected score = 1100 and 5 losses, followed by game over. Restart with `N`, note high score remains 1100. Unplug/replug the Mega and verify the saved high score still shows on the LCD.
4. Test the release-gate motion separately with no balls or pinch hazards: first opening 15 seconds after start, closing after 400 ms; later openings every 15 seconds. Verify that **both gates close** after the fifth lost-ball event.
5. Only integrate the separate Uno once you've enabled the Uno's game input and verified the OFF state on both boards, including startup, reset and disconnected-wire behavior.

To disable Serial test shortcuts after bring-up, set `ALLOW_SERIAL_TEST_COMMANDS = false` in `Config.h`.

## Things to confirm on your real machine

- Pop-bumper **sensor type** (microswitch or IR) and driver ratings / supply and coil specifications.
- Actual servo arm travel and whether a 400 ms gate motion passes **one** ball, not multiple. A time-only gate doesn't guarantee one ball; mechanical singulation may be required.
- Scoring values (100/200/300/500 are chosen example values).
- LCD interface (this version is for **parallel 16x2**, not an I2C backpack).
- Your Uno must run its *separate* Uno sketch, updated to use the Mega's game-enable signal. The Mega does **not** contain the flipper/stepper code.

## Verification scope

Each `.ino`/`.cpp` file was checked with `g++` using stand-in Arduino APIs. A mock execution simulated start, all three score triggers, bumper score, a held losing sensor (only one loss), five separate losses, gate closure, game-disable, EEPROM high score and new game restart. **This is not a compilation in Arduino IDE and not a hardware test.** Electrical functionality, precise servo timing under load, memory footprint, and safe operating limits must be validated using your hardware.
