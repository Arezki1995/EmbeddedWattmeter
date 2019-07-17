#ifndef __SERIAL_USB
#define __SERIAL_USB

	//---This is an average sampling rate from multiple trials (experiences)
	//---the duration of asynchronous reading with DMA is a hard to calculate
	#define AVG_SAMPLING_RATE 666625.0

	//-- NB blocks To visalize
	#define ONE_SEC   	2604
	#define ONE_MILISEC	3

	//--- YOU MUST NEVER CHANGE THIS
	#define BLOCK_SIZE 	512
	
	// reads blocks of current measurements from USB depending on pinctrl configuration and selected channel
	u_int8_t*  readCurrentRawValues(int fd, size_t numberOfBlocks);
	
	// ADC values are read one byte at a type although a signle measurement is 2 bytes
	// this function assembles every two successive bytes into a 16bits measurement value.
	u_int16_t* formatRawMeasurements(u_int8_t* table,size_t numberOfBlocks);

	// Exports the measurement window as a grapher data file to be plotted
	int 	   writeGrapherDataFile(u_int16_t* measures, size_t numberOfBlocks);
	
	// Exports the measuremet window as a CSV
	int		   writeToCSV(u_int16_t* measures,size_t numberOfBlocks);

	// Displays raw measurement
	void       displayMeasures(u_int16_t* measures,size_t numberOfBlocks);

	// displayConfiguration
	void 	   displayConfiguration();
#endif