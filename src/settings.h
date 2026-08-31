#pragma once

#include <stdbool.h>
#include <stdint.h>

#define STATION_ID_MAX_LENGTH 15u
#define WIFI_SSID_MAX_LENGTH 32u
#define WIFI_PASSWORD_MAX_LENGTH 63u

typedef struct {
    char station_id[STATION_ID_MAX_LENGTH + 1u];
    char wifi_ssid[WIFI_SSID_MAX_LENGTH + 1u];
    char wifi_password[WIFI_PASSWORD_MAX_LENGTH + 1u];
    uint8_t keep_alive_enabled;
    uint8_t transmit_enabled;
    uint16_t cw_wpm;
    uint16_t cw_tone_hz;
    uint8_t audio_gain_percent;
    uint16_t warble_low_hz;
    uint16_t warble_high_hz;
    uint16_t warble_switch_ms;
    uint16_t warble_duration_ms;
    uint16_t sweep_low_hz;
    uint16_t sweep_high_hz;
    uint16_t sweep_step_hz;
    uint16_t sweep_step_ms;
    uint16_t sweep_duration_ms;
    uint16_t fox_pause_ms;
    uint16_t tone_pause_ms;
    uint16_t idle_ms;
} fox_settings_t;

void settings_init(void);
void settings_get(fox_settings_t *destination);
bool settings_set(const fox_settings_t *candidate);

// Writes a pending web update to flash. Call only from the main execution
// context, preferably while PTT is off. Returns true if nothing is pending or
// the save succeeded.
bool settings_save_if_dirty(void);
