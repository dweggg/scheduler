# Scheduler

A minimalist, portable, cooperative task scheduler written in C. Designed for bare‑metal environments in which interrupt nesting could become nasty.
It requires only the provided `.c`/`.h` pair. It's lightweight enough to run tasks at several kHz in ~80MHz CPUs.

---

## 📋 Requirements

* A periodic tick source with known frequency (like a hardware timer). You could use DWT/SysTick in a Cortex-M, Core Timer CP0 in PIC32, set up a CPU timer in C28x... A faster tick source will give you the ability to run tasks at higher frequencies.

---

## 🖥️ Example: STM32 (ARM Cortex‑M4)

Below is a complete `main.c` demonstrating integration on an STM32 using the DWT cycle counter as the tick source.

```c

#include "scheduler.h" // don't forget this!

// 1) Enable and zero the DWT cycle counter
static void dwt_init(void) {
    // 1a) Enable trace & debug blocks
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // 1b) Reset the cycle counter
    DWT->CYCCNT = 0;
    // 1c) Enable the cycle counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// 2) Our tick‐source function: just return the 32‑bit cycle counter
static uint32_t dwt_get_ticks(void) {
    return DWT->CYCCNT;
}

uint32_t task_a_freq = 10000; // task a will run at 10kHz, or once every 100us
void task_a(void) {
    // place your code here
}


uint32_t task_b_freq = 300; // task b will run at 200Hz, or once every 5000us
void task_b(void) {
    // place your other code here

}

int main(void) {
    // Initialize DWT
    dwt_init();

    // Initialize scheduler with the tick source
    scheduler_init(
        dwt_get_ticks, // this is the pointer to a function that returns a uint32_t (always increasing counter)
        HAL_RCC_GetSysClockFreq() // this is a uint32_t that indicates the number of ticks generated per second by your tick source
    );

    // Schedule tasks
    scheduler_add_task(task_a, task_a_freq);
    scheduler_add_task(task_b, task_b_freq);

    // Start the scheduler (never returns)
    scheduler_run();

    // Should never reach here
    while (1) {
  }
}
```

You can also modify the task frequencies at runtime and start/stop them. If you need to run tasks slower than 1s you could do something like this:

```
void task_10s(void){
  // your code here
}
void task_2s(void){
  // your code here
}
uint32_t task_1s_freq = 1; //[Hz]
void task_1s(void) {
  static uint32_t counter_1s = 0;

  if (counter_1s % 10 == 0) {
    task_10s();  // runs every 10 seconds
  }
  if (counter_1s % 2 == 0) {
    task_2s();   // runs every 2 seconds
  }

  counter_1s++;
}
```

---

## 🌐 Adapting to Your Platform

To use this scheduler on non‑STM32 or non‑ARM platforms read this:
   * Implement `uint32_t get_ticks(void)` using any hardware timer, just make it return the timer's 32-bit counter. It would be ideal if you are able to run or tap a counter at your CPU speed such as DWT.
   * Ensure it rolls over gracefully (32‑bit wraparound, sawtooth shape) and increments at a constant rate.
   * Set `ticks_per_second` to your timer/CPU frequency.
---

## 📄 License

Do whatever you want.
