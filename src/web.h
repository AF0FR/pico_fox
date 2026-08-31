#pragma once

#include <stdbool.h>

bool web_init(void);

// Set by the web handler and consumed at an off-air boundary.
bool web_reboot_requested(void);
