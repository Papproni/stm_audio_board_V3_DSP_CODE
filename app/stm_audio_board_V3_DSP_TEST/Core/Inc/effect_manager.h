#ifndef EFFECT_MANAGER_H
#define EFFECT_MANAGER_H

#ifdef __cplusplus
 extern "C" {
#endif

// effect_manager > loops > effects
#define MAX_SERIES_SLOT_SIZE 	8
#define MAX_PARALEL_SLOT_SIZE 	3

 typedef struct loops_st{
	uint8_t series_slot_size_u8;	// Number of effects
	uint8_t parallel_slot_size_u8;	// Number of parallel processing

	// Effect pedal callback functions stored in array
	uint32_t effect_callbacks_ap[MAX_SERIES_SLOT_SIZE][MAX_PARALEL_SLOT_SIZE]; //effect_callbacks[series_slot_size_u8][parallel_slot_size_u8]
 	// functions
 	int8_t (*callback) (struct loops_tst* self);
 }loops_tst;

 /*
  * This lib mangages effect Order on Hardware level.
  */


typedef struct effect_managers_st{
	uint8_t 				loop_queue_size_u8; 			// Defines the number of effects between Loops
	uint32_t 				pedalboard_max_exec_time_u32; 	// The used effect maximum execution time
	uint32_t 				maximum_execution_time_u32;		// The maximum amount of time between loops



	delay_parameters_tst 	parameters_st;

	// functions
	int8_t (*callback) (struct effect_managers_st* self);



}effect_managers_tst;

effect_managers_tst effect_manager;

void effect_manager_init();

#endif
