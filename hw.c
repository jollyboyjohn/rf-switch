#include "hw.h"

// Map GPIO, Interrupt and Timer to pointers
void hw_init(volatile unsigned **gpio, volatile unsigned **intr, volatile unsigned **time) {
    int mem_fd;
    void *gpio_map;
    void *intr_map;
    void *time_map;

    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
        printf("can't open /dev/mem \n");
        exit(-1);
    }
 
    gpio_map = mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, GPIO_BASE); 
    intr_map = mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, INTR_BASE); 
    time_map = mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, TIME_BASE); 
 
    close(mem_fd);
 
    if (gpio_map == MAP_FAILED || intr_map == MAP_FAILED || time_map == MAP_FAILED) {
        printf("mmap error\n");
        exit(-1);
    }

    *gpio = (volatile unsigned *)gpio_map;
    *intr = (volatile unsigned *)intr_map;
    *time = (volatile unsigned *)time_map;
    ++(*time); // shift by 4 bytes
}

/* 
 GPIO Actions
 Note: gpio_set_inputs needs to be run before gpio_set_output
*/
// To prepare for Rx
void gpio_set_inputs(volatile unsigned **gpio, int gpio_pin) {
    *( *gpio + ((gpio_pin)/10) ) &= ~(7<<(((gpio_pin)%10)*3));
}
// To prepare for Tx
void gpio_set_outputs(volatile unsigned **gpio, int gpio_pin) {
    *( (*gpio) + ((gpio_pin)/10)) |= (1<<(((gpio_pin)%10)*3));
}

// Tx HIGH (Logical 1 / ON)
void gpio_pull_high(volatile unsigned **gpio, int gpio_pin) {
    *( (*gpio) + 7) = 1<<gpio_pin;
}

// Tx LOW (Logical 0 / OFF)
void gpio_pull_low(volatile unsigned **gpio, int gpio_pin) {
    *( (*gpio) + 10) = 1<<gpio_pin;
}

// Rx 
int gpio_get_level(volatile unsigned **gpio, int gpio_pin) {
    return (*((*gpio)+13) & (1<<gpio_pin));
}

// Switches off 1, 2 and BASIC interrupts for better timing
int block_intr(volatile unsigned **intr, int flag) {
    static unsigned sav132 = 0;
    static unsigned sav133 = 0;
    static unsigned sav134 = 0;

    // Interrupts to be disabled
    if(flag == 1) {
        // Interrupts already disabled
        if(sav132 != 0)
            return(0);

        // Interrupt pending
        if( (*((*intr)+128) | *((*intr)+129) | *((*intr)+130)) != 0)
            return(0);

	// Copy, then disable interrupts (1, 2 & BASIC)
	sav132 = *((*intr)+132); 
        *((*intr)+135) = sav132;
        sav133 = *((*intr)+133);
        *((*intr)+136) = sav133;
        sav134 = *((*intr)+134);
        *((*intr)+137) = sav134;
    } else {
        // Interrupts already enabled
        if(sav132 == 0)
            return(0);
	// Re-enable interrupts (1, 2 & BASIC)
        *((*intr)+132) = sav132;
        *((*intr)+133) = sav133;
        *((*intr)+134) = sav134;
        sav132 = 0;  
    }
    return(1);
}

// Wait and watch the internal 1MHz timer
unsigned time_wait(volatile unsigned **time, unsigned delay, unsigned *last) {
    unsigned target;

    if ((*last) > 0)
	target = (*last) + delay; // use time from relative point used before
    else 
        target = (**time) + delay; // otherwise use a fresh starting point

    while((((**time)-target) & 0x80000000) != 0);  // delay loop

    // return the time, so it can be used as a relative point
    return target;
}
