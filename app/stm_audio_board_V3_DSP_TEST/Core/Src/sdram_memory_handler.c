


#include "sdram_memory_handler.h"
#include "arm_math.h"


float32_t samples[4000] __attribute__((section(".sdram_section")));

sdram_handler_tst sdram_st;

void clear_sdram_memory(){
   sdram_st.current_ptr = 0;
   sdram_st.remaining_memory_u32 = SDRAM_SIZE;
   samples[0] = 15.0;
};
uint32_t sdram_malloc_float32_t_array(uint32_t size){
   return samples;
}


