/* scheduler.c */
#include "scheduler.h"
#include <stddef.h>

static SchedTask_t   sched_tasks[SCHED_MAX_TASKS];
static SchedTickSrc_t sched_tick_source;
static uint32_t       sched_ticks_per_sec;

// CPU‐usage window tracking
static uint32_t       sched_window_start;
static uint32_t       sched_idle_loops;
static uint32_t       sched_total_loops;
static uint8_t        sched_last_cpu_pct;

// dummy counter to burn cycles when idle
static uint32_t       sched_dummy;

void scheduler_init(SchedTickSrc_t tick_source, uint32_t ticks_per_sec) {
    sched_tick_source   = tick_source;
    sched_ticks_per_sec = ticks_per_sec;

    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++) {
        sched_tasks[i].task_fn      = NULL;
        sched_tasks[i].period_ticks = 0;
        sched_tasks[i].last_tick    = 0;
        sched_tasks[i].exec_count   = 0;
        sched_tasks[i].enabled      = 0;
    }

    sched_window_start = sched_tick_source();
    sched_idle_loops   = 0;
    sched_total_loops  = 0;
    sched_last_cpu_pct = 0;
}

int scheduler_add_task(SchedTaskFn_t fn, uint32_t freq_hz) {
    if (fn == NULL || freq_hz == 0) {
        return SCHED_FAILURE;
    }

    // Compute ticks per invocation (round up to at least 1)
    uint32_t period = sched_ticks_per_sec / freq_hz;
    if (period == 0) {
        period = 1;
    }

    uint32_t now = sched_tick_source();
    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++) {
        if (sched_tasks[i].task_fn == NULL) {
            sched_tasks[i].task_fn      = fn;
            sched_tasks[i].period_ticks = period;
            sched_tasks[i].last_tick    = now;
            sched_tasks[i].exec_count   = 0;
            sched_tasks[i].enabled      = 1; // start enabled
            return SCHED_SUCCESS;
        }
    }
    return SCHED_FAILURE;
}

void scheduler_run(void) {
    while (1) {
        sched_total_loops++;
        uint32_t now = sched_tick_source();

        SchedTask_t *next = NULL;
        for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++) {
            SchedTask_t *t = &sched_tasks[i];
            if (t->task_fn == NULL || !t->enabled) continue;
            if ((now - t->last_tick) >= t->period_ticks) {
                if (next == NULL || t->period_ticks < next->period_ticks) {
                    next = t;
                }
            }
        }

        if (next) {
            uint32_t start = sched_tick_source();
            next->task_fn();
            uint32_t end = sched_tick_source();

            // Calculate execution time in microseconds
            uint32_t delta_ticks = end - start;
            next->last_exec_us = (delta_ticks * 1000000U) / sched_ticks_per_sec;

            next->last_tick += next->period_ticks;
            next->exec_count++;
        } else {
            sched_dummy++;
            sched_idle_loops++;
        }

        if ((now - sched_window_start) >= sched_ticks_per_sec) {
            if (sched_total_loops > 0) {
                uint32_t busy = sched_total_loops - sched_idle_loops;
                sched_last_cpu_pct = (uint8_t)((busy * 100U) / sched_total_loops);
            } else {
                sched_last_cpu_pct = 0;
            }
            sched_window_start = now;
            sched_idle_loops   = 0;
            sched_total_loops  = 0;
        }
    }
}

uint32_t scheduler_get_tick(void) {
    return (sched_tick_source ? sched_tick_source() : 0U);
}

int scheduler_set_task_frequency(SchedTaskFn_t fn, uint32_t freq_hz) {
    if (fn == NULL || freq_hz == 0) {
        return SCHED_FAILURE;
    }
    uint32_t new_period = sched_ticks_per_sec / freq_hz;
    if (new_period == 0) {
        new_period = 1;
    }
    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++) {
        if (sched_tasks[i].task_fn == fn) {
            sched_tasks[i].period_ticks = new_period;
            return SCHED_SUCCESS;
        }
    }
    return SCHED_FAILURE;
}

uint32_t scheduler_get_last_exec_time_us(SchedTaskFn_t fn) {
    if (!fn) return 0;
    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++) {
        if (sched_tasks[i].task_fn == fn) {
            return sched_tasks[i].last_exec_us;
        }
    }
    return 0;
}

uint8_t scheduler_get_cpu(void) {
    return sched_last_cpu_pct;
}

int scheduler_start_task(SchedTaskFn_t fn) {
    if (!fn) return SCHED_FAILURE;
    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++) {
        if (sched_tasks[i].task_fn == fn) {
            sched_tasks[i].enabled = 1;
            return SCHED_SUCCESS;
        }
    }
    return SCHED_FAILURE;
}

int scheduler_stop_task(SchedTaskFn_t fn) {
    if (!fn) return SCHED_FAILURE;
    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++) {
        if (sched_tasks[i].task_fn == fn) {
            sched_tasks[i].enabled = 0;
            return SCHED_SUCCESS;
        }
    }
    return SCHED_FAILURE;
}
