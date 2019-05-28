#include "audioMixer_template.h"
#include "server.h"

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>


int main(void)
{

	pthread_t beatThreadID, udpThreadID;

	//create threads with selected function
	pthread_create(&beatThreadID, NULL, AudioMixer_Thread, NULL);
	pthread_create(&udpThreadID, NULL, serverThread, NULL);

	//wait for threads
	pthread_join(beatThreadID, NULL);
	pthread_join(udpThreadID, NULL);

	return 0;
}