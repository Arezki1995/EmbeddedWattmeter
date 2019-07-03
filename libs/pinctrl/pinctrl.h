#ifndef _PINCTRL
#define _PINCTRL

	#define _GET_VALUE(STATE_REG,i) (((STATE_REG & (0x01<<i)) )>>i)
	
	#define ADC_MODE_MSK  0x0F
	
	#define ADC_MODE_A0   	0x00
	#define ADC_MODE_A1   	0x01
	#define ADC_MODE_A2   	0x02
	#define ADC_MODE_A3   	0x03
	#define ADC_MODE_A4   	0x04
	#define ADC_MODE_A5   	0x05
	#define ADC_MODE_A6   	0x06
	#define ADC_MODE_A7   	0x07
	#define ADC_MODE_A8   	0x08
	#define ADC_MODE_A9   	0x09
	#define ADC_MODE_A10  	0x0A
	#define ADC_MODE_A11  	0x0B
	
	#define ADC_MODE_F1   0x0C
	#define ADC_MODE_F2   0x0D
	#define ADC_MODE_F3   0x0E
	#define ADC_MODE_F4   0x0F
	

	  
	//To Do
#endif