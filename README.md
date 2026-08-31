# PicoFox

PicoFox is a Raspberry Pi Pico/Pico W audio fox controller for an FM radio. It
keys PTT on GP15 and generates mic-level tone audio with PWM on GP16.

The repeating sequence is:

1. `FOX` in Morse at 18 WPM, then 1 second off air
2. 700/1100 Hz warble for 5 seconds, then 5 seconds off air
3. `FOX` in Morse, then 1 second off air
4. 500-1500-500 Hz sweep for 5 seconds, then 5 seconds off air
5. `AF0FR AF0FR` in Morse, then 15 seconds off air

Change `STATION_ID` in `src/config.h` to the station actually operating the
transmitter. All timing, tone, GPIO, and PTT polarity settings are in that file.

## Connections

| Pico | Function | Interface requirement |
|---|---|---|
| GP15 | PTT | Drive a 2N3904 or optocoupler interface; do not connect an unknown radio PTT voltage directly |
| GP16 | Audio | Feed the mic input through DC blocking, attenuation, and level adjustment |
| GND | Common | Connect only when the chosen radio interface uses a common ground |
| LED | TX indicator | On while PTT is asserted |

The GP16 PWM signal is 3.3 V logic-level square wave, not mic-level audio.
Start with strong attenuation and increase the level only until deviation is
clean. A simple starting interface is a 1 uF coupling capacitor followed by a
10 kOhm series resistor and a 1 kOhm trimmer to ground, with the trimmer wiper
feeding the mic input. Verify the requirements for your particular radio first.

Use a transistor or optocoupler for PTT. With the common 2N3904 low-side PTT
circuit, GP15 drives the base through roughly 4.7 kOhm, the emitter goes to
ground, and the collector goes to the radio's PTT line. `PTT_ACTIVE_LEVEL` then
remains `1`.

## Build

Install the ARM GNU toolchain, CMake, Ninja (or Make), and the Raspberry Pi Pico
SDK. Initialize the SDK submodules, then either set `PICO_SDK_PATH` or place the
SDK in a `pico-sdk` directory beside this README.

```powershell
cmake -S . -B build -G Ninja -DPICO_BOARD=pico
cmake --build build
```

Hold BOOTSEL while connecting the Pico, then copy `build/PicoFox.uf2` to the
`RPI-RP2` drive. For a Pico W, configure with `-DPICO_BOARD=pico_w` instead.

## On-air use

Test first into a dummy load or with the radio's transmitter disabled. Confirm
the PTT polarity, audio level, frequency, duty cycle, and station ID before
connecting an antenna. The operator is responsible for authorization, station
identification, permitted frequencies, and unattended-transmitter rules in the
country where it is used.
