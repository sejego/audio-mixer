#ifndef MUSIC_RECEIVER_H
#define MUSIC_RECEIVER_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <sys/stat.h>	// for fifo
#include <sys/types.h>	// for fifo
#include <unistd.h>
#include <pthread.h>
#include "udpclient.h"

#define BROADCAST_ADDRESS		"10.255.255.255"
#define BROADCAST_PORT 			7891

void *receive_packets(void *ptr);
void *music_rcv_thread(void *ptr);

#endif