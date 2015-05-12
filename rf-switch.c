#include "rf-switch.h"
#include "hw.h"

volatile unsigned *gpio;
volatile unsigned *intr;
volatile unsigned *time;

static int tx_one[4] = { SHORT, LONG, SHORT, LONG };
static int tx_zero[4] = { SHORT, LONG, LONG, SHORT };
static int tx_sync[2] = { SHORT, SYNC };

/*
Sends 12-bit Datagram:
+---------+---------+-----+-----+
| Channel | Sockets | N/A | ON? |
| 1 2 3 4 | 1 2 3 4 | ? ? | I O |
+---------+---------+-----+-----+
Examples:
| 0 0 0 1 | 1 0 0 0 | 0 0 | 1 0 | - Switch ON: channel IV, socket 1
| 1 0 0 0 | 0 1 0 0 | 0 0 | 0 1 | - Switch OFF: channel I, socket 2 
*/


// Raspberry Pi GPIO
static const int bcm_gpio = 18;

void txPulses(int seq[], int count, unsigned *start) {
    int i;
    char x = 1;
    for (i=0; i<count; i++) {
        if (x)
	    gpio_pull_high(&gpio, bcm_gpio);
	else 
	    gpio_pull_low(&gpio, bcm_gpio);
	
	(*start) = time_wait(&time, seq[i], start);
        x = !x; // flip the bit
    }
}

void sendCmd (int data) {
    int i, b;
    unsigned start = 0;

    for (i=0; i<REPEAT; i++) {
        for (b=0; b<DATAGRAM; b++) {
            if ((data) & (1<<(b)))
		txPulses(tx_one, sizeof(tx_one)/sizeof(int), &start);
	    else 
                txPulses(tx_zero, sizeof(tx_zero)/sizeof(int), &start);
        }
    	txPulses(tx_sync, sizeof(tx_sync)/sizeof(int), &start);
    }
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s -c <n> -s <n> -p <n>\n", name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -c <n>:    channel number (1-4)\n");
    fprintf(stderr, "    -s <n>:    socket number (1-4)\n");
    fprintf(stderr, "    -p <n>:    set power state (0 = off, 1 = on)\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int data = 0;
    int opt; 
    int chan, skt, pwr;

    if(argc != 7) {
       usage(argv[0]);
    }

    while ((opt = getopt(argc, argv, "c:s:p:")) != -1) {
	switch (opt) {
	    case 'c':
		chan = atoi(optarg);
                if (chan < 1 || chan > 4)
		    usage(argv[0]);
		break;
	    case 's':
		skt = atoi(optarg);
                if (skt < 1 || skt > 4)
		    usage(argv[0]);
	        break;
	    case 'p':
		pwr = atoi(optarg);
                if (pwr < 0 || pwr > 1)
		    usage(argv[0]);
		break;
	    default:
		usage(argv[0]);
		break;
	}
    }

    printf("Channel: %d, Socket: %d, Power: %d\n", chan, skt, pwr);

    data += (1<<(chan - 1));
    data += (1<<(skt + 3));

    if (pwr)
    	data += (1<<10);
    else 
 	data += (1<<11);

    sleep(1);

    hw_init(&gpio, &intr, &time);
    gpio_set_inputs(&gpio, bcm_gpio);
    gpio_set_outputs(&gpio, bcm_gpio);
    block_intr(&intr, 1);

    sendCmd(data);

    block_intr(&intr, 0);

    exit(0);
}
