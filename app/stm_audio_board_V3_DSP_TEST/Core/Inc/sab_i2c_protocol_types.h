#ifndef INC_SAB_I2C_PROTOCOL_TYPES_H
#define INC_SAB_I2C_PROTOCOL_TYPES_H

#include "stdint.h"

// THis file contains the communication basics for the I2C protocoll
// between the DSP and DISPLAY unit
enum SAB_I2C_REGS{
	SAB_I2C_REG_PRESETNUM = 1,
	SAB_I2C_REG_LOOP1FX,
	SAB_I2C_REG_LOOP2FX,
	SAB_I2C_REG_LOOP3FX,
	SAB_I2C_REG_LOOP4FX,
	SAB_I2C_REG_FXPARAM1 ,
	SAB_I2C_REG_FXPARAM2 ,
	SAB_I2C_REG_FXPARAM3 ,
	SAB_I2C_REG_FXPARAM4 ,
	SAB_I2C_REG_FXPARAM5 ,
	SAB_I2C_REG_FXPARAM6 ,
	SAB_I2C_REG_FXPARAM7 ,
	SAB_I2C_REG_FXPARAM8 ,
	SAB_I2C_REG_FXPARAM9 ,
	SAB_I2C_REG_FXPARAM10,
	SAB_I2C_REG_FXPARAM11,
	SAB_I2C_REG_FXPARAM12,
	SAB_I2C_REG_INFO,
	SAB_I2C_REG_SAVEPRESET,
	SAB_I2C_REG_LOOPBYPASSSTATE,
	SAB_I2C_REG_NUM_OF_IMPLEMENTED_EFFECTS,
	SAB_I2C_REG_IMPLEMENTED_EFFECTS,
	SAB_I2c_REG_CURRENT_FX
};
// #define SAB_I2C_REG_PRESETNUM        1
#define SAB_I2C_REG_PRESETNUM_LEN    	4
// LoopNFX definitions
// #define SAB_I2C_REG_LOOP1FX          2
// #define SAB_I2C_REG_LOOP2FX          3
// #define SAB_I2C_REG_LOOP3FX          4
// #define SAB_I2C_REG_LOOP4FX          5
#define SAB_I2C_REG_LOOPFX_LEN    		39

// FxParams definitions (1 to 12)
// #define SAB_I2C_REG_FXPARAM1         6
// #define SAB_I2C_REG_FXPARAM2         7
// #define SAB_I2C_REG_FXPARAM3         8
// #define SAB_I2C_REG_FXPARAM4         9
// #define SAB_I2C_REG_FXPARAM5         10
// #define SAB_I2C_REG_FXPARAM6         11
// #define SAB_I2C_REG_FXPARAM7         12
// #define SAB_I2C_REG_FXPARAM8         13
// #define SAB_I2C_REG_FXPARAM9         14
// #define SAB_I2C_REG_FXPARAM10        15
// #define SAB_I2C_REG_FXPARAM11        16
// #define SAB_I2C_REG_FXPARAM12        17
#define SAB_I2C_REG_FXPARAM_LEN      	7

// Other definitions
// #define SAB_I2C_REG_INFO             18
#define SAB_I2C_REG_INFO_LEN             10

// #define SAB_I2C_REG_SAVEPRESET       19
#define SAB_I2C_REG_SAVEPRESET_LEN       1

// #define SAB_I2C_REG_LOOPBYPASSSTATE  20
#define SAB_I2C_REG_LOOPBYPASSSTATE_LEN  3


// SIZE DEFINES
#define SLOT_NAME_MAX_LEN				10

// PRESETS ----------------------------------------------
typedef enum{
	BTN_OFF,
	BTN_ON
}btn_state_ten;

typedef union{
	uint32_t all_u32;
	struct{
		uint8_t preset_Major_u8;
		uint8_t preset_Minor_u8;
		btn_state_ten BTN3_state_en;
		btn_state_ten BTN4_state_en;
	};
}sab_preset_num_tun;

// LOOPS ----------------------------------------------
typedef enum{
	FX_STATE_ON,
	FX_STATE_OFF
}fx_state_ten;

typedef struct{
	fx_state_ten fx_state_en;
	char name[10];
	uint8_t color[3];
}fx_data_tst;

typedef union{
	uint8_t all_pau8[SAB_I2C_REG_LOOPFX_LEN];
	struct{
		fx_data_tst slot1;
		fx_data_tst slot2;
		fx_data_tst slot3;
	};
}sab_loop_num_tun;

// FX params ----------------------------------------------
typedef enum{
	PARAM_TYPE_UNUSED,
	PARAM_TYPE_POT,
	PARAM_TYPE_BTN
}param_type_ten;

typedef union{
	uint8_t all_au8[SAB_I2C_REG_FXPARAM_LEN];
	struct{
		param_type_ten type_en;
		uint8_t 	value_u8;
		char		name[5];
	};
}sab_fx_param_tun;


// INFO ----------------------------------------------
typedef struct{
	uint8_t dsp_fw_ver_major_u8;
	uint8_t dsp_fw_ver_minor_u8;
	uint8_t dsp_update_date_year_u8;
	uint8_t dsp_update_date_month_u8;
	uint8_t dsp_update_date_day_u8;
}fw_info_tst;

typedef union{
	uint8_t all_u8[12];
	struct{
		fw_info_tst   dsp_info_st;
		fw_info_tst   display_info_st;
		btn_state_ten BTN3_state_en;
		btn_state_ten BTN4_state_en;
	};
}sab_info_tun;


// SAVE ----------------------------------------------

typedef union{
	uint8_t all_u8[SAB_I2C_REG_SAVEPRESET_LEN];
	struct{
		uint8_t   save_command;
	};
}sab_save_tun;

// LoopBypassState ----------------------------------------------

typedef union{
	uint8_t all_u8[SAB_I2C_REG_LOOPBYPASSSTATE_LEN];
	struct{
		uint8_t   L12;
		uint8_t   L23;
		uint8_t   L34;
	};
}sab_loopbypass_tun;

#endif /* INC_SAB_I2C_PROTOCOL_TYPES_H */
