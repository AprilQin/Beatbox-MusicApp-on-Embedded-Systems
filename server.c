#include "server.h"


#define PORT 12345
#define BUFSIZE 1000 //almost max size of a udp packet through ethernet
#define CLEAR 0 //value to for memset
#define response_size1 4 //none and rock takes 4 bytes
#define response_size2 6 //custom is 6 bytes
#define response_size3 17 //status is 6 bytes

char* beatmodes[3] = {"none", "rock", "custom"};

void server_help(int s, struct sockaddr* cliaddr, socklen_t addrlen){

	char* return_message = malloc( sizeof(char) * BUFSIZE );

    memset(return_message, CLEAR, BUFSIZE);

    strcpy(return_message, "Accepted command examples:\n");

    strcat(return_message, "mode none -- change to off mode\n");

    strcat(return_message, "mode rock -- change to rock mode\n");

    strcat(return_message, "mode custom -- change to custom mode\n");

    strcat(return_message, "volumeup -- turn volume up by 5.\n");

    strcat(return_message, "volumedw -- turn volume down by 5.\n");

    strcat(return_message, "tempoup -- turn tempo up by 5.\n");

    strcat(return_message, "tempodw -- turn tempo down by 5.\n");

    strcat(return_message, "play hihat -- play hihat sound.\n");

    strcat(return_message, "play base -- play hihat sound.\n");

    strcat(return_message, "play snare -- play hihat sound.\n");

    strcat(return_message, "play kick -- play hihat sound.\n");

    strcat(return_message, "stop -- cause the server program to end.\n\n");

    sendto(s, return_message, BUFSIZE, MSG_CONFIRM, cliaddr, addrlen);

	free(return_message);
}

void server_none(int s, struct sockaddr* cliaddr, socklen_t addrlen){
	mode = 1; //1 is none mode
	sendto(s, beatmodes[0], response_size1, MSG_CONFIRM, cliaddr, addrlen);
	printf("mode none\n");
}
void server_rock(int s, struct sockaddr* cliaddr, socklen_t addrlen){
	mode = 2; //2 is rock mode
	sendto(s, beatmodes[1], response_size1, MSG_CONFIRM, cliaddr, addrlen);
	printf("mode rock\n");
}
void server_custom(int s, struct sockaddr* cliaddr, socklen_t addrlen){
	mode = 3; //1 is custom mode
	sendto(s, beatmodes[2], response_size2, MSG_CONFIRM, cliaddr, addrlen);
	printf("mode custom\n");
}

void server_volumeup(int s, struct sockaddr* cliaddr, socklen_t addrlen){
	AudioMixer_setVolume(volume+5); //assumes each increment or decrement is by 5
	char* return_message = malloc( sizeof(char) * BUFSIZE );
	memset(return_message, CLEAR, BUFSIZE);
	sprintf(return_message, "%i", volume);
	sendto(s, return_message, response_size1, MSG_CONFIRM, cliaddr, addrlen);
	free(return_message);
	printf("volume +5\n");
}

void server_volumedw(int s, struct sockaddr* cliaddr, socklen_t addrlen){
	AudioMixer_setVolume(volume-5);
	char* return_message = malloc( sizeof(char) * BUFSIZE );
	memset(return_message, CLEAR, BUFSIZE);
	sprintf(return_message, "%i", volume);
	sendto(s, return_message, response_size1, MSG_CONFIRM, cliaddr, addrlen);
	free(return_message);
	printf("volume -5\n");
}

void server_tempoup(int s, struct sockaddr* cliaddr, socklen_t addrlen){
	AudioMixer_setBPM(BPM+5);
	char* return_message = malloc( sizeof(char) * BUFSIZE );
	memset(return_message, CLEAR, BUFSIZE);
	sprintf(return_message, "%i", BPM);
	sendto(s, return_message, response_size1, MSG_CONFIRM, cliaddr, addrlen);
	free(return_message);
	printf("tempo +5\n");
}

void server_tempodw(int s, struct sockaddr* cliaddr, socklen_t addrlen){
	AudioMixer_setBPM(BPM-5);
	char* return_message = malloc( sizeof(char) * BUFSIZE );
	memset(return_message, CLEAR, BUFSIZE);
	sprintf(return_message, "%i", BPM);
	sendto(s, return_message, response_size1, MSG_CONFIRM, cliaddr, addrlen);
	free(return_message);
	printf("tempo -5\n");
}


void get_status(int s, struct sockaddr* remaddr, socklen_t addrlen){
	//creat a buff
	const int max_length = 1024;
	char buff[max_length]; 

	//open the file
	FILE *f = fopen("/proc/uptime", "r");
	if (f == NULL)
		printf("file opening error\n");
	
	//read the return the value
	fgets(buff, max_length, f); 
	sendto(s, buff, response_size3, MSG_CONFIRM, remaddr, addrlen);
	fclose(f);
	printf("buff %s\n", buff);
}


void* serverThread(void* arg){
	
	//Create_socket();

	struct sockaddr_in myaddr; // my address 
	struct sockaddr_in cliaddr; // client address
	socklen_t addrlen = sizeof(cliaddr); // length of addresses
	int reclen; //# of bytes received
	int s; // socket 
	
	//create socket and bind socket to port 12345
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
		perror("create socket failed\n"); 
		return 0; 
	} 
	memset((char *)&myaddr, CLEAR, sizeof(myaddr)); 
	myaddr.sin_family = AF_INET; 
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	myaddr.sin_port = htons(PORT); 
	if (bind(s, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) { 
		perror("bind socket failed"); 
		return 0; 
	}

	printf("Server successfully running on port 12345!\n");

	//listening to incoming packets
	while(!stopping){
		char* buf = malloc(sizeof(char) * BUFSIZE); /* receive buffer */ /* create a UDP socket */ 
		reclen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&cliaddr, &addrlen);
		if (reclen > 0) {
			buf[reclen] = '\0';
			printf("%s received\n", buf);
			if ( strcmp(buf, "\n") == 0 ) continue;
			
			else if	( strcmp(buf, "help\n") == 0 )	server_help( s, (struct sockaddr *)&cliaddr, addrlen);
			
			else if	( strcmp(buf, "mode none\n") == 0 )  server_none( s, (struct sockaddr *)&cliaddr, addrlen);

			else if	( strcmp(buf, "mode rock\n") == 0 ) server_rock(s, (struct sockaddr *)&cliaddr, addrlen);

			else if	( strcmp(buf, "mode custom\n") == 0 ) server_custom(s, (struct sockaddr *)&cliaddr, addrlen);

			else if	( strcmp(buf, "volumeup\n") == 0 ) server_volumeup(s, (struct sockaddr *)&cliaddr, addrlen);

			else if	( strcmp(buf, "volumedw\n") == 0 ) server_volumedw(s, (struct sockaddr *)&cliaddr, addrlen);

			else if	( strcmp(buf, "tempoup\n") == 0 )server_tempoup(s, (struct sockaddr *)&cliaddr, addrlen);

			else if	( strcmp(buf, "tempodw\n") == 0 ) server_tempodw( s, (struct sockaddr *)&cliaddr, addrlen);
			
			else if	( strcmp(buf, "play hihat\n") == 0 ){
				Audio_playFile(handle,&Hihat);
			}
			else if	( strcmp(buf, "play snare\n") == 0 ){
				Audio_playFile(handle,&Snare);
			}
			else if	( strcmp(buf, "play base\n") == 0 ){
				Audio_playFile(handle,&Base);
			} 

			else if	( strcmp(buf, "play kick\n") == 0 ){
				Audio_playFile(handle,&Kick);
			}

			else if	( strcmp(buf, "play airdrum1\n") == 0 ){
				Audio_playFile(handle,&airDrumX);
			}

			else if	( strcmp(buf, "play airdrum2\n") == 0 ){
				Audio_playFile(handle,&airDrumY);
			}

			else if	( strcmp(buf, "play airdrum3\n") == 0 ){
				Audio_playFile(handle,&airDrumZ);
			}
			
			else if	( strcmp(buf, "get uptime\n") == 0 ) get_status( s, (struct sockaddr *)&cliaddr, addrlen);

			else if ( strcmp(buf, "stop\n") == 0 )	{
				stopping = true;
				AudioMixer_cleanup();
			}
				
		}
		free(buf);
		
	}
	return NULL;
}
