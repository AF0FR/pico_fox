#pragma once

// Station identification. Change this before transmitting.
#define STATION_ID "KB0TLL"

// Raspberry Pi Pico GPIO assignments.
#define PTT_PIN   15u
#define AUDIO_PIN 16u

// Set to 0 if the PTT interface is active-low at the Pico GPIO.
#define PTT_ACTIVE_LEVEL 1

// Morse settings.
#define CW_WPM     18u
#define CW_TONE_HZ 500u

// Warble settings.
#define WARBLE_LOW_HZ      300u
#define WARBLE_HIGH_HZ     800u
#define WARBLE_SWITCH_MS   140u
#define WARBLE_DURATION_MS 5000u

// Triangle sweep settings.
#define SWEEP_LOW_HZ      300u
#define SWEEP_HIGH_HZ     800u
#define SWEEP_STEP_HZ     20u
#define SWEEP_STEP_MS     20u
#define SWEEP_DURATION_MS 5000u

// Off-air intervals.
#define FOX_PAUSE_MS   1000u
#define TONE_PAUSE_MS  5000u
#define FINAL_PAUSE_MS 15000u

// Radio key-up and key-down protection.
#define PTT_LEAD_MS 150u
#define PTT_TAIL_MS 100u
