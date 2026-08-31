#include "audio.h"

#include <stdbool.h>

#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

#include "config.h"

static uint audio_slice;
static uint audio_channel;

void audio_init(void)
{
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    audio_slice = pwm_gpio_to_slice_num(AUDIO_PIN);
    audio_channel = pwm_gpio_to_channel(AUDIO_PIN);
    pwm_set_enabled(audio_slice, false);
}

void audio_start_tone(uint32_t frequency_hz)
{
    const uint16_t wrap = 4095u;
    const float divider = (float)clock_get_hz(clk_sys) /
                          ((float)frequency_hz * (float)(wrap + 1u));

    pwm_set_enabled(audio_slice, false);
    pwm_set_clkdiv(audio_slice, divider);
    pwm_set_wrap(audio_slice, wrap);
    pwm_set_chan_level(audio_slice, audio_channel, (wrap + 1u) / 2u);
    pwm_set_counter(audio_slice, 0u);
    pwm_set_enabled(audio_slice, true);
}

void audio_stop(void)
{
    pwm_set_enabled(audio_slice, false);
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(AUDIO_PIN, GPIO_OUT);
    gpio_put(AUDIO_PIN, 0);
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
}

void audio_play_warble(void)
{
    const absolute_time_t deadline = make_timeout_time_ms(WARBLE_DURATION_MS);
    bool use_high_tone = false;

    while (!time_reached(deadline)) {
        audio_start_tone(use_high_tone ? WARBLE_HIGH_HZ : WARBLE_LOW_HZ);
        use_high_tone = !use_high_tone;

        const int64_t remaining_ms = absolute_time_diff_us(get_absolute_time(), deadline) / 1000;
        sleep_ms(remaining_ms < WARBLE_SWITCH_MS ? (uint32_t)remaining_ms : WARBLE_SWITCH_MS);
    }

    audio_stop();
}

void audio_play_sweep(void)
{
    const absolute_time_t deadline = make_timeout_time_ms(SWEEP_DURATION_MS);
    int frequency = SWEEP_LOW_HZ;
    int step = SWEEP_STEP_HZ;

    while (!time_reached(deadline)) {
        audio_start_tone((uint32_t)frequency);

        const int64_t remaining_ms = absolute_time_diff_us(get_absolute_time(), deadline) / 1000;
        sleep_ms(remaining_ms < SWEEP_STEP_MS ? (uint32_t)remaining_ms : SWEEP_STEP_MS);

        frequency += step;
        if (frequency >= (int)SWEEP_HIGH_HZ) {
            frequency = SWEEP_HIGH_HZ;
            step = -(int)SWEEP_STEP_HZ;
        } else if (frequency <= (int)SWEEP_LOW_HZ) {
            frequency = SWEEP_LOW_HZ;
            step = SWEEP_STEP_HZ;
        }
    }

    audio_stop();
}
