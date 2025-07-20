// scheduler.h
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#define SCHED_MAX_TASKS       10
#define SCHED_SUCCESS         1
#define SCHED_FAILURE         0
#define SCHED_CPU_WINDOW_US   1000000U  // not used externally

typedef void     (*SchedTaskFn_t)(void);
typedef uint32_t (*SchedTickSrc_t)(void);

typedef struct {
    SchedTaskFn_t  task_fn;
    uint32_t       period_ticks;
    uint32_t       last_tick;
    uint32_t       exec_count;
    uint32_t       last_exec_us;
} SchedTask_t;

/**
 * @brief Initialize the scheduler with your tick source and tick rate.
 * @param tick_source   Function returning current tick count.
 * @param ticks_per_sec Number of ticks your source generates per second.
 */
void scheduler_init(SchedTickSrc_t tick_source, uint32_t ticks_per_sec);

/**
 * @brief Add a periodic task by specifying its frequency in Hz.
 * @param fn        The task function to call.
 * @param freq_hz   How many times per second to run it.
 * @return SCHED_SUCCESS if added; SCHED_FAILURE on error (full, zero freq, null fn).
 */
int scheduler_add_task(SchedTaskFn_t fn, uint32_t freq_hz);

/**
 * @brief Run the scheduler forever.
 */
void scheduler_run(void);

/**
 * @brief Get the current tick count.
 */
uint32_t scheduler_get_tick(void);

/**
 * @brief Change an existing task’s frequency.
 * @param fn        The task function to identify it.
 * @param freq_hz   New frequency in Hz.
 * @return SCHED_SUCCESS if found+updated; SCHED_FAILURE otherwise.
 */
int scheduler_set_task_frequency(SchedTaskFn_t fn, uint32_t freq_hz);

/**
 * @brief Get the last execution time (in microseconds) of a task.
 * @param fn  Task function to query.
 * @return Last execution time in microseconds, or 0 if task not found.
 */
uint32_t scheduler_get_last_exec_time_us(SchedTaskFn_t fn);

/**
 * @brief Get last-measured CPU usage as an integer percent (0–100).
 */
uint8_t scheduler_get_cpu(void);

#endif // SCHEDULER_H
