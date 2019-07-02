#ifndef __SERIAL_USB
#define __SERIAL_USB

	//---This is an average sampling rate from multiple trials (experiences)
	//---the duration of asynchronous reading with DMA is a hard to calculate
	#define AVG_SAMPLING_RATE 666625.0

	//-- NB blocks To visalize
	#define FOR_ONE_SEC   	2604
	#define FOR_ONE_MILISEC	3

	#define BLOCKS_NB   1

	//--- YOU MUST NEVER CHANGE THIS
	#define BLOCK_SIZE 	512

	
	u_int8_t*  readBlocksFromUSB();
	u_int16_t* formatToMeasures(u_int8_t* table);
	void displayMainBuffer(u_int8_t* table);
	void displayMeasures(u_int16_t* measures);
	void displayMeasuresWithUnits(u_int16_t* measures);
	int  writeToBinaryFile(u_int16_t* measures);
#endif