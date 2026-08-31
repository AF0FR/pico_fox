#pragma once

#include <stdbool.h>

void radio_init(void);
bool radio_ptt_on(void);
void radio_ptt_off(void);
void radio_force_ptt_off(void);
void radio_pause_ms(unsigned milliseconds);
