#include "web.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dhcpserver.h"
#include "dnsserver.h"
#include "lwip/apps/httpd.h"
#include "lwip/ip4_addr.h"
#include "pico/cyw43_arch.h"

#include "config.h"
#include "keep_alive.h"
#include "settings.h"
#include "station_control.h"
#include "workflow.h"

static dhcp_server_t dhcp_server;
static dns_server_t dns_server;
static volatile bool reboot_requested;

static void copy_text(char *destination, size_t destination_size,
                      const char *source, bool uppercase)
{
    size_t i = 0;
    for (; source[i] != '\0' && i + 1u < destination_size; ++i) {
        destination[i] = uppercase ?
            (char)toupper((unsigned char)source[i]) : source[i];
    }
    destination[i] = '\0';
}

static const char *parameter_value(int count, char *names[], char *values[],
                                   const char *wanted)
{
    for (int i = 0; i < count; ++i) {
        if (strcmp(names[i], wanted) == 0) {
            return values[i];
        }
    }
    return NULL;
}

static void update_u16(uint16_t *field, const char *name, int count,
                       char *names[], char *values[])
{
    const char *value = parameter_value(count, names, values, name);
    if (value != NULL) {
        *field = (uint16_t)strtoul(value, NULL, 10);
    }
}

static const char *save_handler(int index, int count, char *names[], char *values[])
{
    (void)index;
    fox_settings_t settings;
    settings_get(&settings);

    const char *station_id = parameter_value(count, names, values, "id");
    if (station_id != NULL) {
        copy_text(settings.station_id, sizeof(settings.station_id), station_id, true);
    }

    const char *ssid = parameter_value(count, names, values, "ssid");
    if (ssid != NULL) {
        copy_text(settings.wifi_ssid, sizeof(settings.wifi_ssid), ssid, false);
    }
    const char *password = parameter_value(count, names, values, "password");
    if (password != NULL && password[0] != '\0') {
        copy_text(settings.wifi_password, sizeof(settings.wifi_password), password, false);
    }

    if (parameter_value(count, names, values, "kap") != NULL) {
        settings.keep_alive_enabled =
            parameter_value(count, names, values, "ka") != NULL;
    }

    update_u16(&settings.cw_wpm, "wpm", count, names, values);
    update_u16(&settings.cw_tone_hz, "cw", count, names, values);
    uint16_t gain = settings.audio_gain_percent;
    update_u16(&gain, "gain", count, names, values);
    settings.audio_gain_percent = gain <= UINT8_MAX ? (uint8_t)gain : UINT8_MAX;
    update_u16(&settings.warble_low_hz, "wl", count, names, values);
    update_u16(&settings.warble_high_hz, "wh", count, names, values);
    update_u16(&settings.warble_switch_ms, "ws", count, names, values);
    update_u16(&settings.warble_duration_ms, "wd", count, names, values);
    update_u16(&settings.sweep_low_hz, "sl", count, names, values);
    update_u16(&settings.sweep_high_hz, "sh", count, names, values);
    update_u16(&settings.sweep_step_hz, "ss", count, names, values);
    update_u16(&settings.sweep_step_ms, "sm", count, names, values);
    update_u16(&settings.sweep_duration_ms, "sd", count, names, values);
    update_u16(&settings.fox_pause_ms, "fp", count, names, values);
    update_u16(&settings.tone_pause_ms, "tp", count, names, values);
    update_u16(&settings.idle_ms, "idle", count, names, values);

    if (!settings_set(&settings)) {
        return "/settings.shtml?error=1";
    }
    keep_alive_set_enabled(settings.keep_alive_enabled != 0u);
    return "/settings.shtml?saved=1";
}

static const char *reboot_handler(int index, int count, char *names[], char *values[])
{
    (void)index;
    (void)count;
    (void)names;
    (void)values;
    reboot_requested = true;
    return "/reboot.shtml";
}

static const char *station_handler(int index, int count, char *names[], char *values[])
{
    (void)index;
    fox_settings_t settings;
    settings_get(&settings);
    const char *run = parameter_value(count, names, values, "run");
    const bool enabled = run != NULL && strcmp(run, "1") == 0;
    settings.transmit_enabled = enabled;
    if (settings_set(&settings)) {
        station_control_set_enabled(enabled);
    }
    return "/index.shtml";
}

static const tCGI cgi_handlers[] = {
    {"/save.cgi", save_handler},
    {"/reboot.cgi", reboot_handler},
    {"/station.cgi", station_handler},
};

static const char *ssi_tags[] = {
    "id", "wpm", "cw", "gain", "wl", "wh", "ws", "wd",
    "sl", "sh", "ss", "sm", "sd", "fp", "tp", "idle", "ssid", "kacheck",
    "txstatus", "startdis", "stopdis", "step", "stepid"
};

static u16_t ssi_handler(int index, char *output, int output_length)
{
    fox_settings_t s;
    settings_get(&s);

    switch (index) {
        case 0: return (u16_t)snprintf(output, output_length, "%s", s.station_id);
        case 1: return (u16_t)snprintf(output, output_length, "%u", s.cw_wpm);
        case 2: return (u16_t)snprintf(output, output_length, "%u", s.cw_tone_hz);
        case 3: return (u16_t)snprintf(output, output_length, "%u", s.audio_gain_percent);
        case 4: return (u16_t)snprintf(output, output_length, "%u", s.warble_low_hz);
        case 5: return (u16_t)snprintf(output, output_length, "%u", s.warble_high_hz);
        case 6: return (u16_t)snprintf(output, output_length, "%u", s.warble_switch_ms);
        case 7: return (u16_t)snprintf(output, output_length, "%u", s.warble_duration_ms);
        case 8: return (u16_t)snprintf(output, output_length, "%u", s.sweep_low_hz);
        case 9: return (u16_t)snprintf(output, output_length, "%u", s.sweep_high_hz);
        case 10: return (u16_t)snprintf(output, output_length, "%u", s.sweep_step_hz);
        case 11: return (u16_t)snprintf(output, output_length, "%u", s.sweep_step_ms);
        case 12: return (u16_t)snprintf(output, output_length, "%u", s.sweep_duration_ms);
        case 13: return (u16_t)snprintf(output, output_length, "%u", s.fox_pause_ms);
        case 14: return (u16_t)snprintf(output, output_length, "%u", s.tone_pause_ms);
        case 15: return (u16_t)snprintf(output, output_length, "%u", s.idle_ms);
        case 16: return (u16_t)snprintf(output, output_length, "%s", s.wifi_ssid);
        case 17: return (u16_t)snprintf(output, output_length, "%s",
                                        s.keep_alive_enabled ? "checked" : "");
        case 18: return (u16_t)snprintf(output, output_length, "%s",
                                        station_control_stop_requested() ? "Stopping" :
                                        (station_control_is_enabled() ? "Running" : "Stopped"));
        case 19: return (u16_t)snprintf(output, output_length, "%s",
                                        station_control_is_enabled() ? "disabled" : "");
        case 20: return (u16_t)snprintf(output, output_length, "%s",
                                        station_control_is_enabled() &&
                                        !station_control_stop_requested() ? "" : "disabled");
        case 21: return (u16_t)snprintf(output, output_length, "%s",
                                        workflow_name(workflow_get()));
        case 22: return (u16_t)snprintf(output, output_length, "%u",
                                        (unsigned)workflow_get());
        default: return 0;
    }
}

bool web_init(void)
{
    fox_settings_t settings;
    settings_get(&settings);

    if (cyw43_arch_init() != 0) {
        return false;
    }

    cyw43_arch_enable_ap_mode(settings.wifi_ssid, settings.wifi_password,
                              CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t gateway;
    ip4_addr_t netmask;
    gateway.addr = PP_HTONL(CYW43_DEFAULT_IP_AP_ADDRESS);
    netmask.addr = PP_HTONL(CYW43_DEFAULT_IP_MASK);

    cyw43_arch_lwip_begin();
    dhcp_server_init(&dhcp_server, &cyw43_state.netif[CYW43_ITF_AP],
                     &gateway, &netmask);
    dns_server_init(&dns_server, &cyw43_state.netif[CYW43_ITF_AP], &gateway);
    httpd_init();
    http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
    http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
    cyw43_arch_lwip_end();
    return true;
}

bool web_reboot_requested(void)
{
    return reboot_requested;
}
