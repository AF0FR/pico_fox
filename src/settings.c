#include "settings.h"

#include <stdbool.h>
#include <string.h>

#include "hardware/flash.h"
#include "hardware/regs/addressmap.h"
#include "pico/flash.h"
#include "pico/critical_section.h"

#include "config.h"

static critical_section_t settings_lock;
static fox_settings_t current_settings;
static bool settings_dirty;

#define SETTINGS_MAGIC   0x50465832u  // "PFX2"
#define SETTINGS_VERSION 5u
#define SETTINGS_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t payload_size;
    fox_settings_t payload;
    uint32_t checksum;
} settings_record_t;

typedef struct {
    uint32_t offset;
    uint8_t page[FLASH_PAGE_SIZE];
} flash_write_t;

_Static_assert(sizeof(settings_record_t) <= FLASH_PAGE_SIZE,
               "Settings record must fit in one flash page");

extern uint8_t __flash_binary_end;

static uint32_t checksum_bytes(const void *data, size_t length)
{
    const uint8_t *bytes = data;
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static bool valid_station_id(const char *text)
{
    size_t length = 0;
    while (text[length] != '\0' && length <= STATION_ID_MAX_LENGTH) {
        const char c = text[length];
        const bool valid = (c >= 'A' && c <= 'Z') ||
                           (c >= '0' && c <= '9') || c == '/';
        if (!valid) {
            return false;
        }
        ++length;
    }
    return length > 0 && length <= STATION_ID_MAX_LENGTH;
}

static bool valid_wifi_text(const char *text, size_t minimum, size_t maximum)
{
    size_t length = 0;
    while (length <= maximum && text[length] != '\0') {
        ++length;
    }
    if (length < minimum || length > maximum) {
        return false;
    }
    for (size_t i = 0; i < length; ++i) {
        const unsigned char c = (unsigned char)text[i];
        if (c < 0x20u || c > 0x7eu) {
            return false;
        }
    }
    return true;
}

static bool valid_wifi_ssid(const char *text)
{
    size_t length = 0;
    while (length <= WIFI_SSID_MAX_LENGTH && text[length] != '\0') {
        const unsigned char c = (unsigned char)text[length];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '_' ||
              c == '.' || c == ' ')) {
            return false;
        }
        ++length;
    }
    return length > 0u && length <= WIFI_SSID_MAX_LENGTH;
}

static bool valid(const fox_settings_t *s)
{
    return valid_station_id(s->station_id) &&
           valid_wifi_ssid(s->wifi_ssid) &&
           valid_wifi_text(s->wifi_password, 8u, WIFI_PASSWORD_MAX_LENGTH) &&
           s->keep_alive_enabled <= 1u &&
           s->transmit_enabled <= 1u &&
           s->cw_wpm >= 5 && s->cw_wpm <= 40 &&
           s->cw_tone_hz >= 200 && s->cw_tone_hz <= 2000 &&
           s->audio_gain_percent >= 5 && s->audio_gain_percent <= 100 &&
           s->warble_low_hz >= 200 && s->warble_low_hz < s->warble_high_hz &&
           s->warble_high_hz <= 2500 &&
           s->warble_switch_ms >= 50 && s->warble_switch_ms <= 1000 &&
           s->warble_duration_ms >= 500 && s->warble_duration_ms <= 30000 &&
           s->sweep_low_hz >= 200 && s->sweep_low_hz < s->sweep_high_hz &&
           s->sweep_high_hz <= 2500 &&
           s->sweep_step_hz >= 1 && s->sweep_step_hz <= 200 &&
           s->sweep_step_ms >= 5 && s->sweep_step_ms <= 500 &&
           s->sweep_duration_ms >= 500 && s->sweep_duration_ms <= 30000 &&
           s->fox_pause_ms <= 30000 && s->tone_pause_ms <= 60000 &&
           s->idle_ms <= 60000;
}

void settings_init(void)
{
    critical_section_init(&settings_lock);
    current_settings = (fox_settings_t) {
        .station_id = STATION_ID,
        .wifi_ssid = WIFI_AP_SSID,
        .wifi_password = WIFI_AP_PASSWORD,
        .keep_alive_enabled = KEEP_ALIVE_ENABLED,
        .transmit_enabled = 1u,
        .cw_wpm = CW_WPM,
        .cw_tone_hz = CW_TONE_HZ,
        .audio_gain_percent = AUDIO_GAIN_PERCENT,
        .warble_low_hz = WARBLE_LOW_HZ,
        .warble_high_hz = WARBLE_HIGH_HZ,
        .warble_switch_ms = WARBLE_SWITCH_MS,
        .warble_duration_ms = WARBLE_DURATION_MS,
        .sweep_low_hz = SWEEP_LOW_HZ,
        .sweep_high_hz = SWEEP_HIGH_HZ,
        .sweep_step_hz = SWEEP_STEP_HZ,
        .sweep_step_ms = SWEEP_STEP_MS,
        .sweep_duration_ms = SWEEP_DURATION_MS,
        .fox_pause_ms = FOX_PAUSE_MS,
        .tone_pause_ms = TONE_PAUSE_MS,
        .idle_ms = IDLE_DURATION_MS,
    };
    settings_dirty = false;

    const settings_record_t *saved =
        (const settings_record_t *)(XIP_BASE + SETTINGS_FLASH_OFFSET);
    if (saved->magic == SETTINGS_MAGIC &&
        saved->version == SETTINGS_VERSION &&
        saved->payload_size == sizeof(saved->payload) &&
        saved->checksum == checksum_bytes(&saved->payload, sizeof(saved->payload)) &&
        valid(&saved->payload)) {
        current_settings = saved->payload;
    }
}

void settings_get(fox_settings_t *destination)
{
    critical_section_enter_blocking(&settings_lock);
    *destination = current_settings;
    critical_section_exit(&settings_lock);
}

bool settings_set(const fox_settings_t *candidate)
{
    if (!valid(candidate)) {
        return false;
    }
    critical_section_enter_blocking(&settings_lock);
    if (memcmp(&current_settings, candidate, sizeof(*candidate)) != 0) {
        current_settings = *candidate;
        settings_dirty = true;
    }
    critical_section_exit(&settings_lock);
    return true;
}

static void write_flash(void *parameter)
{
    const flash_write_t *write = parameter;
    flash_range_erase(write->offset, FLASH_SECTOR_SIZE);
    flash_range_program(write->offset, write->page, sizeof(write->page));
}

bool settings_save_if_dirty(void)
{
    flash_write_t write;
    memset(&write, 0xff, sizeof(write));

    critical_section_enter_blocking(&settings_lock);
    if (!settings_dirty) {
        critical_section_exit(&settings_lock);
        return true;
    }

    settings_record_t record = {
        .magic = SETTINGS_MAGIC,
        .version = SETTINGS_VERSION,
        .payload_size = sizeof(record.payload),
        .payload = current_settings,
    };
    record.checksum = checksum_bytes(&record.payload, sizeof(record.payload));
    memcpy(write.page, &record, sizeof(record));
    write.offset = SETTINGS_FLASH_OFFSET;
    settings_dirty = false;
    critical_section_exit(&settings_lock);

    // Do not erase a sector that the linked firmware image occupies. This can
    // only happen if the program grows to fill essentially the entire flash.
    const uintptr_t binary_end_offset =
        (uintptr_t)&__flash_binary_end - (uintptr_t)XIP_BASE;
    if (binary_end_offset > SETTINGS_FLASH_OFFSET) {
        critical_section_enter_blocking(&settings_lock);
        settings_dirty = true;
        critical_section_exit(&settings_lock);
        return false;
    }

    const int result = flash_safe_execute(write_flash, &write, 1000u);
    if (result != PICO_OK) {
        critical_section_enter_blocking(&settings_lock);
        settings_dirty = true;
        critical_section_exit(&settings_lock);
        return false;
    }
    return true;
}
