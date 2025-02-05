#ifndef INC_SDRAM_MEMORY_HANDLER_H
#define INC_SDRAM_MEMORY_HANDLER_H


#include "stm32h7xx_hal.h"
#include "string.h"
#include "stdint.h"
#include <stdlib.h>

#define SDRAM_ADDRESS_START 0xC0000000
#define SDRAM_SIZE 			0x100000 // 16Mb = 2MB

typedef struct sdram_handler_st{
   uint32_t current_ptr;
   uint32_t remaining_memory_u32;
}sdram_handler_tst;


void clear_sdram_memory();
uint32_t sdram_malloc_float32_t_array(uint32_t size);

#endif /* INC_SDRAM_MEMORY_HANDLER_H */
