#pragma once

const char *web_settings_save_handler(int index, int count,
                                      char *names[], char *values[]);
const char *web_settings_defaults_handler(int index, int count,
                                          char *names[], char *values[]);
const char *web_settings_save_class(void);
const char *web_settings_save_message(void);
