#pragma once

#include <stdbool.h>

void keyer_init(void);
void keyer_run(void);
void keyer_set_web_key(bool dah, bool pressed);
void keyer_release_web_keys(void);
