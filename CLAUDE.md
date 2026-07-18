# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project type

Bare-metal + FreeRTOS firmware for the TI **TM4C123GH6PM** (ARM Cortex-M4, Tiva C LaunchPad), built with **Keil MDK-ARM / uVision5**. There is no CMake/Makefile/CLI build system — the project file is `Lab3.uvprojx`.

## Build

Requires Keil uVision5 (MDK-ARM) with the TI device pack for TM4C123GH6PM, plus **TivaWare_C_Series-2.2.0.295** installed at `C:\ti\TivaWare_C_Series-2.2.0.295` (this exact path is hardcoded in `Lab3.uvprojx`'s include path and the `driverlib.lib`/`gpio.c` file references — moving TivaWare will break the build).

- GUI: open `Lab3.uvprojx` in uVision, Build Target (F7), Download (F8) to flash over the LaunchPad's ICDI/XDS110 debugger.
- CLI (verified working): `UV4.exe -r "Lab3.uvprojx" -j0 -o "build.log"` (`-r` = rebuild all; `-j0` = non-interactive/no popups). `UV4.exe` on this machine lives under Keil's per-user install dir (`%LOCALAPPDATA%\Keil_v5\UV4\UV4.exe`), not on PATH by default. `Start-Process ... -Wait` (not plain `&`) is needed from PowerShell since UV4 is a GUI-subsystem app that otherwise returns immediately; quote all paths as a single argument string, not an array, or spaces in the path get mangled.
- Target device config (from `Lab3.uvprojx`): Cortex-M4, FPU2, 12 MHz crystal, 256KB flash / 32KB RAM.
- There is no automated test suite; validation is manual, over UART (PuTTY, 9600 baud) and observing GPIO/LCD/buzzer behavior on real hardware.

## Architecture

This is a 4-task FreeRTOS application coordinating over three single-slot queues (all created with depth 1 — each queue holds only the latest value, so tasks always act on the most recent reading rather than a backlog):

- `xUARTQueue` (1 byte) — new setpoint, UART task → Main task
- `xLCDQueue` (8 bytes, a 2-field struct) — measured + setpoint text, Main task → LCD task
- `xBuzzerQueue` (1 byte) — alarm on/off flag, Main task → Buzzer task

All four tasks run at the same priority (7); FreeRTOS time-slicing (`configUSE_TIME_SLICING=1`, `configTICK_RATE_HZ=1000`) round-robins them every 1ms tick.

Task responsibilities, all defined in [Trial2.c](Trial2.c) and declared in [Trial2.h](Trial2.h):
- `Main_Task` — reads the LM35 temperature sensor via ADC0 SS3 (polled, PE3/AIN0), compares against the setpoint received from `xUARTQueue`, drives the heater relay + status LED on Port F (with a small hysteresis band), pushes display text to `xLCDQueue`, and pushes the alarm state to `xBuzzerQueue` when temperature exceeds `AlarmValue` (70°C).
- `UART_Task` — prompts over UART0 (9600 baud, PA0/PA1), blocking-reads digits typed by the user until `\n`, accumulates them into an integer setpoint, and sends it via `xUARTQueue`.
- `LCD_Task` — owns the `LCD` struct (from [LCD.h](LCD.h)/[LCD.c](LCD.c)), receives `{Txt1, Txt2}` from `xLCDQueue` and redraws measured/setpoint temperature once per second.
- `Buzzer_Task` — receives the on/off flag from `xBuzzerQueue` and toggles the buzzer GPIO (PF1) accordingly.

`PROJECT_Init()` in Trial2.c performs all one-time hardware bring-up (Port E, Port F, UART0, ADC0) and creates the three queues before `main()` (in [Main0.c](Main0.c)) creates the tasks and starts the scheduler.

### Peripheral-access style (uniform driverlib — keep it that way)

All peripheral access now goes through TI TivaWare `driverlib` (e.g. `GPIOPinWrite`, `SysCtlPeripheralEnable`, `UARTCharGet`, `ADCSequenceDataGet`) across both `Trial2.c` and `LCD.c`. This was **not** always the case: the code originally mixed three styles (raw register macros from `tm4c123gh6pm.h`, CMSIS-style peripheral structs like `GPIOE->AFSEL`, and driverlib) and was unified to driverlib. When touching peripheral code, stay in driverlib rather than reintroducing raw-register or CMSIS-struct access.

Notes for driverlib work here:
- `PART_TM4C123GH6PM` is `#define`d in [Trial2.h](Trial2.h) before including `driverlib/pin_map.h` — required for the pin-mux macros (`GPIO_PA0_U0RX`, etc.) to be visible. Don't remove it.
- Named pin constants (`HEATER_PIN`/`LED_PIN`/`BUZZER_PIN` on Port F, `PORTE_OUTPUT_PINS`, `TEMP_SENSOR_PIN`, `TEMP_ADC_SEQUENCER`) live in `Trial2.h`; use them instead of bit masks.
- UART0 is deliberately clocked from the 16 MHz PIOSC (`UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC)` + `UARTConfigSetExpClk(..., 16000000, 9600, ...)`) so the 9600 baud rate doesn't depend on the system clock — matching the original register-level setup. Don't switch it to `SysCtlClockGet()` without setting the system clock first.
- `tm4c123gh6pm.h` and the CMSIS device header are still in the include set but are no longer used for peripheral access.

### Files not part of the logical task system

`Main0.c` is the live entry point — its `int main()` runs `PROJECT_Init()`, creates the four tasks, and starts the scheduler. `Trial.c` is an earlier standalone LCD test program with its own `int main()` and is **not** included in the Keil build (`Lab3.uvprojx` only lists `LCD.c`, `gpio.c`, `Trial2.c`, `Main0.c` under Source Group 1). Be aware of this before assuming any `.c` file in the root is live code — check `Lab3.uvprojx`'s `<Files>` list to confirm what's actually compiled.

### FreeRTOS pack version sensitivity

The active FreeRTOS config is `RTE/RTOS/FreeRTOSConfig.h` (wired in via `RTE_Components.h` → `RTE_RTOS_FreeRTOS_CONFIG`); a root-level `FreeRTOSConfig.h` used to exist as an unreferenced duplicate and has been deleted. Only the **Config/Core/Heap_1** CMSIS-FreeRTOS RTE components are installed (no Timers component), and `configUSE_TIMERS` is `0` to match — if new code ever needs software timers, both the RTE component and `timers.c` would need to be added, or it'll fail at link time with undefined `xTimerCreateTimerTask`/`xTimerGetTimerDaemonTaskHandle`. `configCHECK_FOR_STACK_OVERFLOW` is `2`, which requires `vApplicationStackOverflowHook` to be defined somewhere in the build (currently in `Main0.c`) — removing it without providing a hook reintroduces a link error. The code originally used the legacy FreeRTOS `xQueueHandle` type alias, which the installed CMSIS-FreeRTOS 11.3.0 pack doesn't define; it's been updated to `QueueHandle_t` in `Trial2.c` — don't reintroduce the old alias.

The build also emits harmless `-Wmacro-redefined` warnings (e.g. `WATCHDOG0_BASE`) from TivaWare's `inc/hw_memmap.h` and the CMSIS device header both defining the same peripheral base addresses with identical values — expected side effect of both header families being in the include set, not a regression to chase.

## Applied fixes / current state (do not regress)

- **`xQueueHandle` → `QueueHandle_t`** (legacy alias not defined by the 11.3.0 pack).
- **`configUSE_TIMERS` set to `0`** and **`vApplicationStackOverflowHook` defined in `Main0.c`** — both required to link.
- **Unused root `FreeRTOSConfig.h` deleted** (only `RTE/RTOS/FreeRTOSConfig.h` is active).
- **`setpoint` initialized to `25`** in `Main_Task` (was uninitialized; the non-blocking queue read left it as stack garbage before the first UART entry).
- **Temperature conversion precedence fixed**: `(int)(mV/10.0)` instead of `(int) mV/10.0`.
- **Peripheral access unified to driverlib** with named pin constants — see the peripheral-access section above.

## Remaining refactor suggestions (not yet done)

1. **Fix LCD peripheral-clock race** in `LCD_setup()` — no wait for `SysCtlPeripheralReady()` after enabling clocks.
2. **Rename dead-or-legacy files**: `Trial.c` (dead), `Main0.c`/`Trial2.c`/`Trial2.h` (legacy names), `Lab3.*` (Keil project name).
3. **Split the monolithic `Trial2.c`** into separate driver files (GPIO, UART, ADC) and a tasks file.
4. **Avoid busy-waiting inside RTOS tasks** — `Main_Task`/`Buzzer_Task` never call `vTaskDelay` and use non-blocking queue reads; only works because `configUSE_TIME_SLICING` is on.
5. **Guard the LCD text buffers** — `Message.Txt1`/`Txt2` are 4-byte arrays with zero margin for a sign or 4th digit.
6. **Validate UART input** — `UART_Task` has no digit check or length bound.
