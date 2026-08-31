#include "pico/stdlib.h"

#include "audio.h"
#include "fox.h"
#include "keep_alive.h"
#include "radio.h"
#include "settings.h"
#include "station_control.h"
#include "web.h"

int main(void)
{
    stdio_init_all();
    settings_init();
    keep_alive_init();
    if (!web_init()) {
        while (true) {
            sleep_ms(1000);
        }
    }
    radio_init();
    station_control_init();
    audio_init();

    sleep_ms(2000);

    while (true) {
        fox_run_cycle();
    }
}
