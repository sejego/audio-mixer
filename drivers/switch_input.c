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
#include "ZedboardOLED.h"
#include "switch_input.h"
/*
https://raspberrypi.stackexchange.com/questions/44416/polling-gpio-pin-from-c-always-getting-immediate-response 
*/

void intialize_equalizers()
{
    const uint32_t coeffs[15] = {11446, 22892, 11446, 2157422138, 1063849117, 122524214, 0, 4172443082, 2489628699,
                                 828693395, 314491699, 3665983898, 314491699, 0, 184224972};
    int IRQEnable;
    int res;                             
    int fd_1 = open ("/dev/uio0", O_RDWR);
    int fd_2 = open ("/dev/uio1", O_RDWR);
    unsigned pageSize = sysconf(_SC_PAGESIZE);
    void *ptr_1;
    void *ptr_2; 
    ptr_1 = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_1, 0); // 0 for 1st base address.    
    ptr_2 = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_2, 0); // 0 for 1st base address.
    for(int i=0; i < 15; i++)
    {
        *((unsigned *)(ptr_1 + 4*i)) = coeffs[i];
        *((unsigned *)(ptr_2 + 4*i)) = coeffs[i];
    }
    *((unsigned *)(ptr_1 + 60)) = 1;
    *((unsigned *)(ptr_1 + 60)) = 0;
    *((unsigned *)(ptr_1 + 64)) = 1;
    *((unsigned *)(ptr_2 + 60)) = 1;
    *((unsigned *)(ptr_2 + 60)) = 0;
    *((unsigned *)(ptr_2 + 64)) = 1;
    IRQEnable = 1; 
    res = write(fd_1, &IRQEnable , sizeof(int));
    res = read(fd_1, &IRQEnable, sizeof(int));
    IRQEnable = 1; 
    res = write(fd_2, &IRQEnable , sizeof(int));
    res = read(fd_2, &IRQEnable, sizeof(int));
    munmap(ptr_1, pageSize);
    munmap(ptr_2, pageSize);
    close(fd_1);
    close(fd_2);
    ptr_1 = NULL;
    ptr_2 = NULL;
}

void initialize_switches()
{
    const char switch_id[MAX_PINS][4]= {
                         "952",
                         "953",
                         "954",
                         "955",
                         "956",
                         "957"
                     };
    int fd;
    char temp[MAX_LEN];
    const char path[] = "/sys/class/gpio/gpio";

    // Export all switches that will be used
    printf("Start exporting Switches\n");
    for(int i = 0; i < MAX_PINS; i++){
        fd = open("/sys/class/gpio/export", O_WRONLY);
	    write(fd, switch_id[i], 3);
	    close(fd);
    }
    printf("Switches exported\n");

    // Set all switches as inputs
    strcpy(temp,path);

    for(int i = 0; i < MAX_PINS; i++){
        strcat(temp, switch_id[i]);
        fd = open(strcat(temp,"/direction"), O_WRONLY);
	    write(fd, "in", 2);
	    close(fd);
        strcpy(temp,path);
    }

    // Add interrupts to every switch to be toggled at both rising
    // and falling edges.

    for(int i = 0; i < MAX_PINS; i++){
        strcat(temp, switch_id[i]);
        fd = open(strcat(temp,"/edge"), O_WRONLY);
	    write(fd, "both", 4);
	    close(fd);
        strcpy(temp,path);
    }
}

void write_eq_reg(unsigned short offset,  uint32_t value, char *uio)
{
    void *ptr;
    int IRQEnable = 1;
    int res = 0;
    int fd = open (uio, O_RDWR);
    unsigned pageSize = sysconf(_SC_PAGESIZE);
    ptr = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    *((uint32_t *)(ptr + offset)) = value;
    res = write(fd, &IRQEnable , sizeof(int));
    res = read(fd, &IRQEnable, sizeof(int));
    munmap(ptr, pageSize);
    close(fd);
    ptr = NULL;
}

void write_to_equalizer(int id, char val, void *addr)
{
    uint32_t val2 = (uint32_t)(val) - 48;
    switch (id) {
        case 0:
            write_eq_reg(68, val2, "/dev/uio1");
            oledPrintChar(val, 3, 11, addr);
            break;
        case 1:
            write_eq_reg(72, val2, "/dev/uio1");
            oledPrintChar(val, 3, 10, addr);
            break;
        case 2:
            write_eq_reg(76, val2, "/dev/uio1");
            oledPrintChar(val, 3, 9, addr);
            break;
        case 3:
            write_eq_reg(68, val2, "/dev/uio0");
            oledPrintChar(val, 3, 6, addr);
            break;
        case 4:
            write_eq_reg(72, val2, "/dev/uio0");
            oledPrintChar(val, 3, 5, addr);
            break;
        case 5:
            write_eq_reg(76, val2, "/dev/uio0");
            oledPrintChar(val, 3, 4, addr);
            break;
        default:
            break;
    }
    oledUpdate(addr);
}


void *start_sw_thread(void *ptr)
{
    void *oledAddress = ((switchThreadCtrl *)ptr)->oledAddr;
    int *ctrl = ((switchThreadCtrl *)ptr)->status;

    const char switch_id[MAX_PINS][4]= {
                         "952",
                         "953",
                         "954",
                         "955",
                         "956",
                         "957"
                     };
    const char path[] = "/sys/class/gpio/gpio";
    char temp[MAX_LEN];
    int temp_fd;
    char out;
    int ret;

    // https://kevinboone.me/pi-button-pipe.html?i=1
    
    struct pollfd fdset[MAX_PINS];

    for(int i=0;i<MAX_PINS;i++){
        strcpy(temp,path);
        strcat(temp,switch_id[i]);
        strcat(temp,"/value");
        temp_fd = open(temp, O_RDONLY|O_NONBLOCK);
        fdset[i].fd = temp_fd;
        fdset[i].events = POLLPRI;
    }

    while(*ctrl)
    {
        poll(fdset, MAX_PINS, 1000);
        for (int i = 0; i < MAX_PINS; i++){
            if (fdset[i].revents & POLLPRI){
                close(fdset[i].fd);
                strcpy(temp,path);
                strcat(temp,switch_id[i]);
                strcat(temp,"/value");
                fdset[i].fd = open(temp, O_RDONLY|O_NONBLOCK);
                read(fdset[i].fd, &out, 1);
                write_to_equalizer(i, out, oledAddress);
            }
        }
    }

    for (int i = 0; i < MAX_PINS; ++i)
    {
        close(fdset[i].fd);
    }
    printf("Switch thread finished\n");
}