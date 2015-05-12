#include <unistd.h>
#include "hw.h"

#define REPEAT 4
#define DATAGRAM 12
#define SHORT 320 
#define LONG 960 
#define SYNC 10240

void txPulses(int seq[], int count, unsigned *start);
void sendCmd (int data);
void usage(char *name);
int main(int argc, char *argv[]);
