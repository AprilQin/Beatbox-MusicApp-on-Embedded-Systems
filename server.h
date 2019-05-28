#ifndef _SERVER_H_
#define _SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#include "audioMixer_template.h"

void server_help(int s, struct sockaddr* cliaddr, socklen_t addrlen);
void server_none(int s, struct sockaddr* cliaddr, socklen_t addrlen);
void server_rock(int s, struct sockaddr* cliaddr, socklen_t addrlen);
void server_custom(int s, struct sockaddr* cliaddr, socklen_t addrlen);
void server_volumeup(int s, struct sockaddr* cliaddr, socklen_t addrlen);
void server_volumedw(int s, struct sockaddr* cliaddr, socklen_t addrlen);
void server_tempoup(int s, struct sockaddr* cliaddr, socklen_t addrlen);
void server_tempodw(int s, struct sockaddr* cliaddr, socklen_t addrlen);
void get_status(int s, struct sockaddr* cliaddr, socklen_t addrlen);
void* serverThread(void* arg);

#endif