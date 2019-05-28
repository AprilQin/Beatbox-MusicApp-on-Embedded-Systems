#include "sleep.h"
#include <stdio.h>
#include <time.h>


void Sleeeep(long s, long ns){
	// printf("sleep for %lu seconds\n", ns);
	struct timespec reqDelay = {s, ns};
	nanosleep(&reqDelay, (struct timespec *) NULL);
}
