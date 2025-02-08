


#include "sdram_memory_handler.h"
#include "arm_math.h"

#define NUM_OF_SAMPLES 512000
float32_t samples[NUM_OF_SAMPLES] __attribute__((section(".sdram_section")));

sdram_handler_tst sdram_st;

void clear_sdram_memory(){
   memset(samples,0,sizeof(samples));
   sdram_st.current_ptr = 0;
};

uint32_t* sdram_malloc_float32_t_array(uint32_t size){
   uint32_t addr = sdram_st.current_ptr;
   sdram_st.current_ptr+=size;
   return &(samples[addr]);
}


