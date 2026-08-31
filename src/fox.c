#include "fox.h"

#include "audio.h"
#include "config.h"
#include "morse.h"
#include "radio.h"

static void transmit_warble(void)
{
    radio_ptt_on();
    audio_play_warble();
    radio_ptt_off();
}

static void transmit_sweep(void)
{
    radio_ptt_on();
    audio_play_sweep();
    radio_ptt_off();
}

void fox_run_cycle(void)
{
    morse_transmit("FOX");
    radio_pause_ms(FOX_PAUSE_MS);

    transmit_warble();
    radio_pause_ms(TONE_PAUSE_MS);

    morse_transmit("FOX");
    radio_pause_ms(FOX_PAUSE_MS);

    transmit_sweep();
    radio_pause_ms(TONE_PAUSE_MS);

    morse_transmit(STATION_ID " " STATION_ID);
    radio_pause_ms(FINAL_PAUSE_MS);
}
