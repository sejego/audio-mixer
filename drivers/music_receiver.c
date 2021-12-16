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
#include "music_receiver.h"
//#define BROADCAST_ADDRESS		"10.255.255.255"
//#define BROADCAST_PORT 			7891

char * fifo = "/tmp/myfifo";
int f_fifo;

void *music_rcv_thread(void *ptr)
{
	int count= 512;
	int16_t samples[count];
	int16_t ssample;
	int32_t sample;
	pthread_t thread_id;

	mkfifo(fifo, 0666);		// why 0666 ? umask?
	int fd_axi_audio = open ("/dev/uio5", O_RDWR);
    int fd_filter = open ("/dev/uio1", O_RDWR);


    unsigned pageSize = sysconf(_SC_PAGESIZE);

    int IRQEnable = 1, IRQCount; 

    void *p_audioIP;
    void *p_filterIP;

    p_audioIP = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_axi_audio, 0); // 0 for 1st base address.
    p_filterIP = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_filter, 0); // 0 for 1st base address.

    if (pthread_create(&thread_id, NULL, receive_packets, ptr) )
    {
    	printf("error creating thread.");
		abort();
    }


    int fifo_read = open(fifo, O_RDONLY);
    while(*((int *)ptr)){
    	read(fifo_read, &samples, sizeof(samples));
    	for(int i=0; i<count; i++){
    		sample = (int32_t)samples[i];
    		*((unsigned *)(p_audioIP+0)) = sample;
    		write(fd_filter, &IRQEnable, sizeof(IRQEnable));	// this may be shit
    		read(fd_filter, &IRQCount, sizeof(IRQCount));
    	}
    }
    pthread_join(thread_id, NULL);
    munmap(p_audioIP, pageSize);
    close(fd_filter);
    close(fd_axi_audio);
    close(fifo_read);
    printf("Music thread exited\n"); 
}

void *receive_packets(void *ptr)
{
	int count= 512; // amount of samples in a packet
	int16_t samples[count];
	udp_client_setup(BROADCAST_ADDRESS, BROADCAST_PORT);
	f_fifo = open(fifo, O_WRONLY);
	while (*((int *)ptr)){
		udp_client_recv(samples,sizeof(samples));
		write(f_fifo, samples, sizeof(samples));		
	}
	close(f_fifo);
	printf("UDP receiver thread exited\n");	
}