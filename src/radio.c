#include "radio.h"

#include "pico/stdlib.h"

#include "audio.h"
#include "config.h"

static inline int ptt_inactive_level(void)
{
    return !PTT_ACTIVE_LEVEL;
}

void radio_init(void)
{
    gpio_init(PTT_PIN);
    gpio_set_dir(PTT_PIN, GPIO_OUT);
    gpio_put(PTT_PIN, ptt_inactive_level());

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}

void radio_ptt_on(void)
{
    gpio_put(PTT_PIN, PTT_ACTIVE_LEVEL);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    sleep_ms(PTT_LEAD_MS);
}

void radio_ptt_off(void)
{
    audio_stop();
    sleep_ms(PTT_TAIL_MS);
    gpio_put(PTT_PIN, ptt_inactive_level());
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}

void radio_pause_ms(unsigned milliseconds)
{
    audio_stop();
    gpio_put(PTT_PIN, ptt_inactive_level());
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    sleep_ms(milliseconds);
}
