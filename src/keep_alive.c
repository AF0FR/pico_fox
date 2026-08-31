#include "keep_alive.h"

#include <stdbool.h>

#include "pico/stdlib.h"

#include "config.h"

// A short timer tick keeps the configured pulse and period easy to tune. The
// callback only writes a GPIO, so it is safe to run from the timer IRQ context.
#define KEEP_ALIVE_TICK_MS 100u

#if KEEP_ALIVE_PULSE_MS == 0
#error "KEEP_ALIVE_PULSE_MS must be greater than zero"
#endif

#if KEEP_ALIVE_PULSE_MS >= KEEP_ALIVE_PERIOD_MS
#error "KEEP_ALIVE_PULSE_MS must be shorter than KEEP_ALIVE_PERIOD_MS"
#endif

#if (KEEP_ALIVE_PULSE_MS % KEEP_ALIVE_TICK_MS) != 0
#error "KEEP_ALIVE_PULSE_MS must be a multiple of KEEP_ALIVE_TICK_MS"
#endif

#if (KEEP_ALIVE_PERIOD_MS % KEEP_ALIVE_TICK_MS) != 0
#error "KEEP_ALIVE_PERIOD_MS must be a multiple of KEEP_ALIVE_TICK_MS"
#endif

static struct repeating_timer keep_alive_timer;
static uint32_t phase_ms;

static void set_load(bool enabled)
{
    gpio_put(KEEP_ALIVE_PIN,
             enabled ? KEEP_ALIVE_ACTIVE_LEVEL : !KEEP_ALIVE_ACTIVE_LEVEL);
}

static bool keep_alive_callback(struct repeating_timer *timer)
{
    (void)timer;

    phase_ms = (phase_ms + KEEP_ALIVE_TICK_MS) % KEEP_ALIVE_PERIOD_MS;
    set_load(phase_ms < KEEP_ALIVE_PULSE_MS);
    return true;
}

void keep_alive_init(void)
{
    gpio_init(KEEP_ALIVE_PIN);
    gpio_set_dir(KEEP_ALIVE_PIN, GPIO_OUT);

    // Begin with a load pulse so the power bank sees it immediately at startup.
    phase_ms = 0u;
    set_load(true);

    // A negative interval schedules relative to the previous start time and
    // prevents cumulative drift in the five-second period.
    add_repeating_timer_ms(-(int32_t)KEEP_ALIVE_TICK_MS,
                           keep_alive_callback,
                           NULL,
                           &keep_alive_timer);
}
