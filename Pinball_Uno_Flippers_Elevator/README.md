# Arduino Uno #1: Flippers + Worm Elevator

Sketch: `Pinball_Uno_Flippers_Elevator.ino`

## Hardware assumptions

- Two flipper buttons, each wired between Uno pin and GND (`INPUT_PULLUP`).
- Two external, correctly rated MOSFET/solenoid driver circuits; active-HIGH control inputs. Arduino Uno pins **do not** power the coils.
- One **STEP/DIR/ENABLE** stepper-motor driver (such as an A4988/DRV8825 in an appropriate compatible configuration), with its own correctly rated motor supply. Arduino Uno pins **do not** power the stepper.
- Appropriate protective circuits for inductive loads, hardware-safe solenoid drive limits, and motor current limiting must be confirmed before using the real mechanisms.

## Pin map

| Component | Uno pin | Note |
|---|---|---|
| Left flipper button | D2 | Button to GND; pressed = LOW |
| Right flipper button | D3 | Button to GND; pressed = LOW |
| Left MOSFET driver input | D5 | HIGH when firing |
| Right MOSFET driver input | D6 | HIGH when firing |
| Optional game-enable signal | D7 | Set `USE_GAME_ENABLE_INPUT=true`; LOW = active |
| Stepper driver STEP | D8 | Rising-edge stepping |
| Stepper driver DIR | D9 | Constant direction |
| Stepper driver ENABLE | D10 | Active LOW on supported driver |

All Arduino-to-driver logic must use a compatible voltage and a shared signal-reference ground **when required by the driver**. Do not connect a high-voltage motor supply to Arduino pins. Verify stepper driver VMOT, motor winding pairs, current limit, microstep pins, and enable polarity from the actual driver's documentation. Do not connect/disconnect the stepper with motor power on.

## Behavior

- Each flipper fires once per press, turns off when its button is released or when the configured pulse interval expires, and cannot fire again until its button is released and pressed again.
- The elevator motor advances continuously without `delay()` using `micros()` for step timing.
- Serial Monitor (9600 baud): send `E` to request elevator ON; send `S` to request OFF.
- `USE_GAME_ENABLE_INPUT=false` makes the board run independently during testing. Set it `true` after connecting a safe, compatible signal from the game's main Arduino to D7: D7 LOW means game enabled, D7 HIGH/unconnected means disabled. Use common signal ground and agree on game signal polarity. The flippers and elevator are both disabled at game over.

## Important configuration

`FLIPPER_PULSE_MS = 50` is **only a placeholder**. It has NOT been validated for your specific solenoid model. In particular, some pinball flipper coils are dual-winding and require a proper end-of-stroke/hold-current arrangement. A timed HIGH signal into a MOSFET alone may not be an appropriate or safe flipper driver. Before powering real coils, confirm the coil part number, supply voltage, driver circuit, switching polarity, current rating and necessary independent safety protection. Test initially with low-power LED indicators (with appropriate resistors) in place of real drivers, not live coils.

`STEP_INTERVAL_US = 5000` is 200 step pulses/second. Actual shaft speed depends on steps per revolution and microstepping mode: RPM = 60,000,000 / (STEP_INTERVAL_US * full_steps_per_revolution * microstep_divisor). Select a rate compatible with your driver, motor, load and mechanical gearing. This constant-speed sketch has no acceleration ramp, obstruction sensing, homing or jam protection, so it is a **baseline**, not a finished elevator controller. For a heavy elevator, add a non-blocking acceleration profile and jam/interlock detection once motor and driver specs are known.

## Hardware protection

Provide a physical power disconnect/emergency stop, correct fusing, rated wiring, suitable flyback/TVS/snubber arrangements as required by the driver, and driver current limits. Motor windings and solenoid coils should never be tied directly to digital outputs. A software pulse limit cannot protect against a stuck transistor or stalled motor.

## Build

Open the folder `Pinball_Uno_Flippers_Elevator` in Arduino IDE, choose **Arduino Uno** and the connected COM/USB port, then **Verify**. No third-party Arduino library is required.
