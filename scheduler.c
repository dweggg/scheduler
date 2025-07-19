// scheduler.c
#include "scheduler.h"
#include <stdlib.h>

static Task_t       task_list[MAX_TASKS];
static TickSource_t get_ticks;
static uint32_t     ticks_per_s;

// CPU usage vars
static uint32_t     window_start_tick;
static uint32_t     idle_count;
static uint32_t     total_loops;
static float        last_cpu;

void scheduler_init(TickSource_t tick_function, uint32_t ticks_per_second) {
    get_ticks   = tick_function;
    ticks_per_s = ticks_per_second;

    for (int i = 0; i < MAX_TASKS; i++) {
        task_list[i].function      = NULL;
        task_list[i].period_ticks  = 0;
        task_list[i].last_run_tick = 0;
        task_list[i].exec_count    = 0;
    }

    window_start_tick = get_ticks();
    idle_count        = 0;
    total_loops       = 0;
    last_cpu          = 0.0f;
}

bool scheduler_add_task(TaskFunction_t function, float period_seconds) {
    if (!function || period_seconds <= 0.0f) return false;

    uint32_t period = (uint32_t)(period_seconds * (float)ticks_per_s);
    uint32_t now    = get_ticks();

    for (int i = 0; i < MAX_TASKS; i++) {
        if (!task_list[i].function) {
            task_list[i].function      = function;
            task_list[i].period_ticks  = period;
            task_list[i].last_run_tick = now;
            task_list[i].exec_count    = 0;
            return true;
        }
    }
    return false;
}

static uint32_t dummy = 0;

void scheduler_run(void) {
    while (1) {
        total_loops++;
        uint32_t now = get_ticks();

        // pick next ready task
        Task_t *next = NULL;
        for (int i = 0; i < MAX_TASKS; i++) {
            Task_t *t = &task_list[i];
            if (!t->function) continue;
            if ((now - t->last_run_tick) >= t->period_ticks) {
                if (!next || t->period_ticks < next->period_ticks) next = t;
            }
        }

        if (next) {
            next->function();
            // catch up by one period to avoid backlog
            next->last_run_tick += next->period_ticks;
        } else {
            dummy++;
            idle_count++;
        }

        // update CPU usage every second based on tick source
        if ((now - window_start_tick) >= ticks_per_s) {
            // idle_ratio = idle loops / total loops in window
            float idle_ratio = total_loops ? ((float)idle_count / (float)total_loops) : 1.0f;
            if (idle_ratio > 1.0f) idle_ratio = 1.0f;
            last_cpu = (1.0f - idle_ratio) * 100.0f;

            // reset for next window
            window_start_tick = now;
            idle_count        = 0;
            total_loops       = 0;
        }
    }
}

uint32_t scheduler_get_tick(void) {
    return get_ticks ? get_ticks() : 0;
}

bool scheduler_set_task_period(TaskFunction_t function, float period_seconds) {
    if (!function || period_seconds <= 0.0f) return false;
    uint32_t new_t = (uint32_t)(period_seconds * (float)ticks_per_s);
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_list[i].function == function) {
            task_list[i].period_ticks = new_t;
            return true;
        }
    }
    return false;
}

float scheduler_get_cpu(void) {
    return last_cpu;
}
