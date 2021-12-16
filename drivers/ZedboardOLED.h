/* 	ZedboardOLED.c
 * 	###############################################################################
 *
 *	Simple driver for the 128x32 OLED on the Zedboard
 *
 *
 *	The presence of the ZedboardOLED_v1_0 IP code in the PL-side of the Zynq chip
 *	is required to make this driver work.
 *
 *
 *	Texas A&M Qatar invests efforts and resources providing this open source code,
 *	All text above, and the splash screen below must be included in any redistribution
 *	this driver is subjected to BSD license, check license.txt for more information
 *	Written by Ali Aljaani/ Texas A&M Qatar University.
 *
 * 	###############################################################################
 * 	v1.0 -- 08/15/2014
 *  Eng. Ali Aljaani
 *  Company: TAMUQ University
 * 	###############################################################################
 *
 * 	Edited by Karl Janson in 04/19/2017:
 * 	1. Added definition of Xil_Out32 so that the driver can be used from Linux
 *  2. Code cleanup
 *
 * 	*/

#ifndef ZEDBOARDOLED_H
#define ZEDBOARDOLED_H

#include <inttypes.h>

#define REG_LENGTH 4
#define CHAR_LENGTH_BITS 8
#define OLED_LINE_LENGTH 16
#define OLED_UPDATE_BIT 64

#define SLEEP_TIME_US 250

static uint32_t displayBuffer[16];

/*****************************************************************************/
/**
*
* clears the screen, example: clean();
*
*
* @param	void *oledBaseAddr: base address for the OLED IP
*
* @return	none.
******************************************************************************/
void oledClear(void *oledBaseAddr);

/*****************************************************************************/
/**
*
* Updates the OLED display
*
* @param	void *oledBaseAddr: base address for the OLED IP
*
******************************************************************************/

void oledUpdate(void *oledBaseAddr);

/*****************************************************************************/
/**
*
* prints a character on the OLED at the page and the position specified by the second
* and third arguments,example print_char('A',0,0);
*
* @param	char character:  the character to be printed.
*
* @param	int row(0-3): the OLED is divided into 4 pages numbers, 0 is the upper
*			3 is the lower.
* @param	int position(0-15): each row can hold 16 characters
* 			0 is the leftmost: 15 is the rightmost
* @param	void *oledBaseAddr: base address for the OLED IP
*
* @return	int , 1 on success , 0 on failure.

******************************************************************************/
int oledPrintChar(char character, int row, int pos, void *oledBaseAddr);

/*****************************************************************************/
/**
*
* prints a string of characters on the OLED at the page specified by the second
* argument, maximum string per page =16,example: print_char("Texas A&M Qatar,0);
*
* @param	char *string , the string message to be printed , maximum 16 letters.
*
* @param	int row(0-3) , the OLED is divided into 4 pages numbers, 0 is the upper
*			3 is the lower.
* @param	void *oledBaseAddr: base address for the OLED IP
*
* @return	int , 1 on success , 0 on failure.
*
******************************************************************************/
int oledPrintString(char *string, int row, void *oledBaseAddr);

#endif // ZEDBOARDOLED_H
