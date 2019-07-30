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
	
	#define ADC_MODE_666K   0x0C
	#define ADC_MODE_280K   0x0D
	#define ADC_MODE_125K   0x0E
	#define ADC_MODE_60K    0x0F
	

	#define NB_PINS 7
	#define VALUE_PATH_SIZE 30
	#define DIRECTION_PATH_SIZE 35
	#define BUFFER_MAX 3
	#define IN   0
	#define OUT  1
	#define LOW  0
	#define HIGH 1

	// command GPIO pins on Rpi
	#define INTERRUPT_PIN 6

	int GPIOExport(int pin);
	int GPIOUnexport(int pin);
	int GPIODirection(int pin, int dir);
	int GPIORead(int pin);
	int GPIOWrite(int pin, int value);
	int EnableCommandPins();
	int SetCommandPinsDirection();
	int writeCommand(int cmd);
	int DisableCommandPins();
	//To Do
#endif