#include "pico/stdlib.h"

#include "audio.h"
#include "fox.h"
#include "keep_alive.h"
#include "radio.h"

int main(void)
{
    stdio_init_all();
    radio_init();
    audio_init();
    keep_alive_init();

    sleep_ms(2000);

    while (true) {
        fox_run_cycle();
    }
}
