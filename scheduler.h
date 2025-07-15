#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_TASKS 10

typedef void     (*TaskFunction_t)(void);
typedef uint32_t (*TickSource_t)(void);

typedef struct {
    TaskFunction_t function;
    uint32_t       period_ticks;
    uint32_t       last_run_tick;
    uint32_t       exec_count;
} Task_t;

/**
 * @brief Initialize the scheduler with a tick source and tick rate.
 */
void scheduler_init(TickSource_t tick_function, uint32_t ticks_per_second);

/**
 * @brief Add a periodic task to the scheduler.
 * @param function Task function to call.
 * @param period_seconds Period in seconds (e.g., 0.01 for 10ms).
 * @return true if task added successfully, false if no room.
 */
bool scheduler_add_task(TaskFunction_t function, float period_seconds);

/**
 * @brief Run the scheduler loop. Never returns.
 */
void scheduler_run(void);

/**
 * @brief Get the current tick value from the scheduler's tick source.
 */
uint32_t scheduler_get_tick(void);

#endif // SCHEDULER_H
