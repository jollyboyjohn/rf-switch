#include <stdio.h>    // printf
#include <stdlib.h>   // exit
#include <fcntl.h>    // file ops
#include <sys/mman.h> // mem ops

#define HWIO_BASE 0x20000000
#define GPIO_BASE (HWIO_BASE + 0x200000)
#define TIME_BASE (HWIO_BASE + 0x3000)
#define INTR_BASE (HWIO_BASE + 0xB000)

#define BLOCK_SIZE (4*1024)

void hw_init(volatile unsigned **gpio, volatile unsigned **intr, volatile unsigned **time);

void gpio_set_inputs(volatile unsigned **gpio, int gpio_pin);
void gpio_set_outputs(volatile unsigned **gpio, int gpio_pin);
void gpio_pull_high(volatile unsigned **gpio, int gpio_pin);
void gpio_pull_low(volatile unsigned **gpio, int gpio_pin);
int gpio_get_level(volatile unsigned **gpio, int gpio_pin);

int block_intr(volatile unsigned **intr, int flag);

unsigned time_wait(volatile unsigned **time, unsigned delay, unsigned *last);
