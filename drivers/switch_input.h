#ifndef SWITCH_INPUT_H
#define SWITCH_INPUT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>

#define MAX_PINS 6
#define MAX_LEN 200

typedef struct switchThreadCtrl{
    void *oledAddr;
    int *status;
}switchThreadCtrl;

void intialize_equalizers();
void initialize_switches();
void write_eq_reg(unsigned short offset,  uint32_t value, char *uio);
void write_to_equalizer(int id, char val, void *addr);
void *start_sw_thread(void *ptr);
#endif