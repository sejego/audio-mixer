#include <inttypes.h>
#include "io_lib.h"

/**
 * Writes a value to a register at baseAddress + offset 
 * @param baseAddress   Base address of the device to access
 * @param offset        Offset of the register to be accessed
 * @param value         Value to be written to the register
 */
void write_reg(void *baseAddress, unsigned short offset,  uint32_t value)
{
    *((uint32_t *)(baseAddress + offset)) = value;
}

/**
 * Reads a value from a register at baseAddress + offset 
 * @param baseAddress   Base address of the device to access
 * @param offset        Offset of the register to be accessed
 * 
 * @return              Value stored in the register
 */
uint32_t read_reg(void *baseAddress, unsigned short offset, uint32_t value)
{
    return *((uint32_t *)(baseAddress + offset));
}