#include "scheduler.h"
#include <stdlib.h>

static Task_t      task_list[MAX_TASKS];
static TickSource_t get_ticks;
static uint32_t    ticks_per_s;

void scheduler_init(TickSource_t tick_function, uint32_t ticks_per_second) {
    get_ticks   = tick_function;
    ticks_per_s = ticks_per_second;

    for (int i = 0; i < MAX_TASKS; i++) {
        task_list[i].function      = NULL;
        task_list[i].period_ticks  = 0;
        task_list[i].last_run_tick = 0;
        task_list[i].exec_count    = 0;
    }
}

bool scheduler_add_task(TaskFunction_t function, float period_seconds) {
    uint32_t period = (uint32_t)(period_seconds * (float)ticks_per_s);
    uint32_t now    = get_ticks();

    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_list[i].function == NULL) {
            task_list[i].function      = function;
            task_list[i].period_ticks  = period;
            task_list[i].last_run_tick = now;
            task_list[i].exec_count    = 0;
            return true;
        }
    }
    return false;
}

void scheduler_run(void) {
    while (1) {
        Task_t *next = NULL;
        uint32_t now = get_ticks();

        for (int i = 0; i < MAX_TASKS; i++) {
            Task_t *t = &task_list[i];
            if (t->function == NULL)
                continue;

            uint32_t delta = now - t->last_run_tick;
            if (delta >= t->period_ticks) {
                if (next == NULL || t->period_ticks < next->period_ticks) {
                    next = t;
                }
            }
        }

        if (next) {
            next->function();
            next->last_run_tick = now;
            next->exec_count++;
        } else {
            // idle hook: could use __WFI(), low-power, or background tasks
        }
    }
}

uint32_t scheduler_get_tick(void) {
    return get_ticks ? get_ticks() : 0;
}
