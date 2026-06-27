# gimbal_mit - DM6220 MIT gimbal controller

Keil project:

```text
MDK-ARM/DM_6220.uvprojx
Target: DM_6220
Output: MDK-ARM/DM_6220/DM_6220.hex
```

## Hardware Map

| Axis | Motor ID | Bus | MCU pins | Power output |
| --- | --- | --- | --- | --- |
| YAW | `0x01` | CAN1 / FDCAN1 | PD0 RX / PD1 TX | PC14 / OUT1 |
| Pitch | `0x02` | CAN2 / FDCAN2 | PB5 RX / PB6 TX | PC13 / OUT2 |

LCD is the 1.69 inch ST7789 screen in landscape mode:

```text
LCD_W = 280
LCD_H = 240
PB3=SCK, PD7=SDA, PE15=CS, PD10=DC, PB11=RES, PB10=BLK
```

The 5-way navigation key follows the `dm-mc02-master/例程/CtrBoard-H7_KEY`
example. It is an ADC resistor-key module, not five independent GPIO inputs.

```text
ADC1 rank 1: PC4 / ADC1_INP4  board voltage sample
ADC1 rank 2: PA5 / ADC1_INP19  5-way key sample
Sampling: polled every 10 ms in GimbalKeys_Task
Key ADC thresholds:
CENTER < 200
DOWN   700..1000
UP     1500..1800
RIGHT  2200..2500
LEFT   2800..3500
```

## Menu Operation

Status screen:

- Center short press: open main menu.
- Center long press, about 800 ms: smooth center home.
- Center very long press, about 2000 ms: emergency stop trajectory and disable both motors.

Menu:

- Up / Down: select item.
- Left: go back.
- Right or Center: enter or confirm.
- In parameter edit: Up/Right increases, Down/Left decreases, Center saves.
- Flash zero commands require confirmation: `CENTER = YES`, `LEFT = NO`.

Main menu pages:

- `Run Mode`: Stop Motors, MIT Circle, Manual Jog, Center Home.
- `MIT Params`: KP, KD, Torque FF, Amp YAW, Amp Pitch, Period, Reset Default.
- `Zero / Calib`: software zero, save YAW zero, save Pitch zero, save both.
- `Motor`: enable/disable, single-axis enable, OUT1/OUT2 power toggle.
- `Display`: backlight on/off and orientation info.
- `About`: CAN ID, wiring and build notes.

## Control Behavior

The main loop still sends MIT frames on a 1 ms tick. LCD refresh is reduced to
50 ms. The menu only modifies `GimbalControlState`; hardware commands are
executed from `main.c`.

Default conservative MIT parameters:

```text
KP = 1.0
KD = 0.08
Torque FF = 0.0
Yaw amp = 10 deg
Pitch amp = 10 deg
Period = 1000 ms
```

Limits:

```text
KP 0..50
KD 0..2
Torque FF -1..1
Amplitude 0..45 deg
Period 500..5000 ms
```

Zero-save safety:

- The menu blocks save-zero if the target motor is offline.
- Confirm page switches mode to `CALIB`, then the save request is executed as
  `CTRL_STOP`.
- Enable/Disable/Save-zero commands are treated as protected motor command
  frames. MIT output is paused while such a request is pending and briefly
  after the command is accepted by FDCAN, so the special CAN frame is not
  immediately overwritten by normal position control frames.
- Put the mechanism at the desired mechanical center before pressing YES.
- Keep `SAVE_ZERO_ON_BOOT` in `Core/Inc/main.h` at `0` for normal operation.
