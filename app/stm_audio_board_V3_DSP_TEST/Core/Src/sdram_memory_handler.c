


#include "sdram_memory_handler.h"
#include "arm_math.h"

#define NUM_OF_SAMPLES 10
float32_t samples[NUM_OF_SAMPLES] __attribute__((section(".sdram_section")));

sdram_handler_tst sdram_st;

void write_to_sdram_memory(){
   if (sdram_st.current_ptr < NUM_OF_SAMPLES){
      samples[sdram_st.current_ptr] = sdram_st.current_ptr;
      sdram_st.current_ptr++;
   }else{
      sdram_st.current_ptr = 0;
   }
};
uint32_t sdram_malloc_float32_t_array(uint32_t size){
   return samples;
}


