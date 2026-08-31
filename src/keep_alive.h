#pragma once

#include <stdbool.h>

// Starts the background USB power-bank keep-alive pulse generator.
void keep_alive_init(void);

// Enables or disables the external load immediately. The web setting remains
// responsible for persistence.
void keep_alive_set_enabled(bool enabled);
