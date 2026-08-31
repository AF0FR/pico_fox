#pragma once

typedef enum {
    WORKFLOW_STOPPED = 0,
    WORKFLOW_FOX_1,
    WORKFLOW_PAUSE_1,
    WORKFLOW_WARBLE,
    WORKFLOW_PAUSE_2,
    WORKFLOW_FOX_2,
    WORKFLOW_PAUSE_3,
    WORKFLOW_SWEEP,
    WORKFLOW_PAUSE_4,
    WORKFLOW_STATION_ID,
    WORKFLOW_IDLE,
    WORKFLOW_STOP_ID,
    WORKFLOW_REBOOT_ID,
} workflow_step_t;

void workflow_set(workflow_step_t step);
workflow_step_t workflow_get(void);
const char *workflow_name(workflow_step_t step);
