/*
* Simple app to read/write into custom IP in PL via /dev/uoi0 interface
* To compile for arm: make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
* ( Based on Kjans Tsotnep's app )
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <pthread.h>
#include "ZedboardOLED.h"
#include "switch_input.h"
#include "music_receiver.h"

void user_volume_selection(int*);
void update_volume_display(void*, int, int);
void write_volume(const char *filename, int val);

int main(int argc, char *argv[])
{
    int loop_state = 1;
    int user_selection = 0;
    int c;
    int hp_volume = 200;
    int eth_volume = 200;
    pthread_t thread_switches;
    pthread_t thread_music;
    switchThreadCtrl *sw_ctrl = (struct switchThreadCtrl *)malloc(sizeof(struct switchThreadCtrl));

    // Open OLED controller file
    int fd = open ("/dev/uio4", O_RDWR);
    if (fd < 1) { perror(argv[0]); return -1; }

    // Get architecture specific page size
    unsigned pageSize = sysconf(_SC_PAGESIZE);

    // Pointer to the virtual address space, where physical memory will be mapped
    void *oledBaseAddr; 

    // Memory Mapping 
    oledBaseAddr = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // 0 for 1st base address.


    // INIT SWITCHES
    intialize_equalizers();
    initialize_switches();


    // INIT VOLUMES
    update_volume_display(oledBaseAddr, hp_volume, eth_volume);

    // Init screen
    char str[16];
    
    sprintf(str, "MIX HP   ETH"); 

    oledPrintString(str, 0, oledBaseAddr);
    sprintf(str, "EQ  BMH  BMH");
    oledPrintString(str, 2, oledBaseAddr);

    sw_ctrl->oledAddr = oledBaseAddr;
    sw_ctrl->status = &loop_state;
    if (pthread_create(&thread_switches, NULL, start_sw_thread, (void *)sw_ctrl) )
    {
        printf("error creating thread_switches.");
        abort();
    }

    if (pthread_create(&thread_music, NULL, music_rcv_thread, &loop_state) )
    {
        printf("error creating thread_music.");
        abort();
    }

    // Control loop
    while(loop_state){

        printf("\nMixer controller\n");
        printf("Opions:\n");
        printf("1 - Headphone volume\n");
        printf("2 - Internet volume\n");
        printf("3 - Exit\n");
        printf("Enter your selection\n");
        
        scanf("%d", &user_selection);
        while ( (c = getchar()) != '\n' && c != EOF ) { }
        
        if(user_selection > 3 || user_selection < 0){
            printf("\nI do not like you too\n");
        } else if (user_selection == 0){
            printf("\nI'm gonna ignore that\n");
        } else {
            printf("User entered %d\n", user_selection);
            switch(user_selection){
                case 1:
                    user_volume_selection(&hp_volume);
                    update_volume_display(oledBaseAddr, hp_volume, eth_volume);
                    write_volume("/dev/uio2", hp_volume);
                    break;
                case 2:
                    user_volume_selection(&eth_volume);
                    update_volume_display(oledBaseAddr, hp_volume, eth_volume);
                    write_volume("/dev/uio3", eth_volume);
                    break;
                case 3:
                    loop_state = 0;
                    break;
            }
        }
        user_selection = 0;
    }
    pthread_join(thread_music, NULL);
    pthread_join(thread_switches, NULL);
    printf("\n==============================\n");
    printf("Threads are joined and ended\n");
    oledClear(oledBaseAddr);
    //unmap
    munmap(oledBaseAddr, pageSize);
    free(sw_ctrl);
    sw_ctrl = NULL;
    //close
    close(fd);
    printf("Exiting...Goodbye!\n");
    return 0;
}

void user_volume_selection(int* volume){
    int user_selection = 0;
    int c;
    while(1){
        printf("\nChoose volume in range 0 - 4096\n");
        printf("Enter your selection\n");
        
        scanf("%d", &user_selection);
        while ( (c = getchar()) != '\n' && c != EOF ) { }
        
        if(user_selection > 4096 || user_selection < 0){
            printf("\nI do not like you too, try again\n");
        } else if (user_selection == 0){
            printf("\nI'm gonna ignore that\n");
        } else {
            *volume = user_selection;
            return;
        }
        user_selection = 0;
    }
}

void update_volume_display(void *oledBaseAddr, int hp_vol, int eth_vol){
    char str[16];

    sprintf(str, "VOL %4d %4d", hp_vol, eth_vol); 
    oledPrintString(str, 1, oledBaseAddr);
}

void write_volume(const char *filename, int val){
    int fd = open (filename, O_RDWR);
    unsigned pageSize = sysconf(_SC_PAGESIZE);
    void *ptr;
    ptr = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    *((unsigned *)(ptr + 0)) = val;
    *((unsigned *)(ptr + 4)) = val;
    munmap(ptr, pageSize);
    close(fd);
    ptr = NULL;
}