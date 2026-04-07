<div align="center">
  <h1>STM32F103 RTC + LCD Menu Firmware</h1>
</div>
<br>

This repository contains firmware for an STM32F103-based clock/alarm device using:
- DS3231 RTC (I2C).
- 16x2 I2C LCD (LiquidCrystal_I2C style).
- Three buttons for menu: UP, DOWN, MENU.
- Buzzer for alarms notification. 

The firmware implements a menu-driven user interface for configuring date, time, and multiple alarms directly on the device.

---

## Overview
![System](assets/System.png)
The firmware implements:
- Main screen showing DATE and TIME (from DS3231).
- A top menu (BACK / DATE / TIME / ALARM).
- Submenus for setting date, setting time and configuring up to 5 alarms.
- Editing UI that moves a cursor (using LCD cursor on) and increments/decrements fields according to the "contro" index.
- Alarm checking and buzzer output when an alarm triggers.
---

## Features

- Read/write RTC (DS3231) via I2C.
- 16x2 I2C LCD UI with cursor used for editing fields.
- Menu system: BACK, SET DATE, SET TIME, ALARM.
- Up to 5 alarms (manual editing UI present).
- Buzzer notification on alarm.
- Cursor-based field editing system

--- 

## Hardware wiring

- STM32F103 (Blue Pill) — I2C1 pins:
  - PB6 -> I2C1 SCL
  - PB7 -> I2C1 SDA
  - (Ensure DS3231 and LCD share the same I2C bus and addresses.)
- Buttons (active-low with MCU internal pull-ups):
  - PA5 -> UP
  - PA6 -> DOWN
  - PA4 -> MENU
  - Buttons other side -> GND
- Buzzer:
  - PA8 -> Buzzer+ (other side -> GND)
- Common ground: connect DS3231, LCD and MCU grounds together.

Note: If using external pull-ups on I2C lines, 4.7kΩ is typical.

---

## Build & Flash (quick)

Prerequisites:
- STM32CubeMX project for STM32F103 (or HAL/Makefile setup).
- arm-none-eabi toolchain.

Typical compile (example from your environment):
- Build object files with arm-none-eabi-gcc (STM32CubeIDE does this automatically).
- Link via arm-none-eabi-ld / via IDE.

If using command line and Makefile:
- Ensure include paths reference:
  - CMSIS device headers
  - Core/Inc
  - HAL driver includes
- If using i2c-lcd.c that references hi2c1, make sure `hi2c1` is defined/declared once.

Flash:
- Use ST-Link (ST-LINK CLI / STM32CubeProgrammer) or your preferred flashing tool.
- If you have a custom bootloader, flash the application to the correct bank/offset.

---

## Usage / UI guide

- Boot: device starts and shows:
  - Line 0: DATE: DD-MM-20YY
  - Line 1: TIME: HH:MM:SS
![Main Screen](assets/Main_Screen.png)

- Menu:
  - Press MENU to open top menu.
  - Use UP/DOWN to move menu cursor.
  - MENU to select.
![Menu](assets/Menu1.png)


- SET DATE:
  - Navigate to DATE -> SETUP DATE.
  - Editing fields are chosen by `contro` index; UP/DOWN change the currently selected field according to original mapping.
  - Confirm/exit as per original sketch behavior (MENU toggles between menu/edit modes).
![Set Date](assets/Set_Date.png)

- SET TIME:
  - Navigate to TIME -> SETUP TIME.
  - Similar editing behavior as SET DATE.
![Set Time](assets/Set_Time.png)

- Alarms:
  - ALARM menu lists up to 5 alarms.
  - Selecting an alarm shows DATE and TIME fields for that alarm.
  - Alarm checking occurs when on the main screen; when condition matches, buzzer pulses.
![Alarm Triggered](assets/Alarm_Triggered.png)

---

## Video
[**Video**](https://www.youtube.com/watch?v=Y7rGqm_2nNY)

<p align="center">
  <a href="https://www.youtube.com/watch?v=Y7rGqm_2nNY">
    <img src="assets/Video.png" alt=""Video" width="500">
  </a>
</p>

---
## License & Credits
<div align="center">

**© 2026 – Ho Chi Minh City University of Technology and Engineering (HCMUTE)**  
**Electronics & Communication Engineering Technology**

**Nguyễn Phạm Huy Hoàng**  

</div>

