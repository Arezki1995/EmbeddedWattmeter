#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h> 
#include <string.h>
#include <fcntl.h>            // IO
#include <sys/types.h>        // u_int16_t
#include "serialUSB.h"

////////////////////////////////////////////////////////////////////////////////////
u_int8_t* readCurrentRawValues(int fd, size_t numberOfBlocks){
		// Reads current measurement from serialUSB
		if (fd <0){
			fprintf(stderr,"[!] readCurrentRawValues: Error opening device ttyACM0\n");
			return NULL;
		} 
		
		u_int8_t* mainBuffer = malloc(numberOfBlocks*BLOCK_SIZE*sizeof(u_int8_t));
		
		if(mainBuffer==NULL){
			fprintf(stderr,"[!] readCurrentRawValues: Memory allocation failed.(maybe too large)\n");
			return NULL;
		}

		//clear Memory region
		//memset(mainBuffer, 0, numberOfBlocks*BLOCK_SIZE);
		
		int bytesRead;
		for (size_t k = 0; k < numberOfBlocks; k++)
		{	
			// The Idea: I have a main Buffer I can read chunk by chunk from DUE 
			// each time i read a chunk I move the storage pointer by a chunk  
			// a chunk is 512 bytes better leave this as default
			bytesRead = read(fd, mainBuffer+(k*BLOCK_SIZE), BLOCK_SIZE);
			usleep(10);

			// Nothing to be read
			if (bytesRead == 0)
			{
				fprintf(stderr,"[!] readCurrentRawValues: data not ready\n");
				close(fd);
				free(mainBuffer);
				return NULL;
			}
			// Couldn't acces the file for read
			if (bytesRead < 0)
			{
				fprintf(stderr,"[!] readCurrentRawValues: read Impossible, verify DUE is connected with native usb\n");
				close(fd);
				free(mainBuffer);
				return NULL;
			}
		}
		return mainBuffer;
}

////////////////////////////////////////////////////////////////////////////////////
u_int16_t* formatRawMeasurements(u_int8_t* table,size_t numberOfBlocks){
	if (table==NULL)
	{
		fprintf(stderr,"[!] formatRawMeasurements: Table to be formatted is Null\n");
		return NULL;
	}else{
		// The main buffer stores a series of bytes read for serialUSB but a displayMeasure
		// has a resolution of 12bits thus we need to combine every two successive bytes
		// to compose a single measure (with the adequate shifting)
		// this results in a table of "u_int16_t" of half the size of the MainBuffer
		u_int16_t* measures = malloc((numberOfBlocks)*(BLOCK_SIZE/2)*sizeof(u_int16_t));
		size_t j=0;	
		for (size_t i = 0; i <BLOCK_SIZE*numberOfBlocks; i+=2)
		{	
			measures[j]= ((table[i+1]&0x00FF)<<8) | ( table[i] & 0x00FF);
			j++;
		}	
		return measures;
	}
}
////////////////////////////////////////////////////////////////////////////////////
int writeGrapherDataFile(u_int16_t* measures, size_t numberOfBlocks){
	if (measures==NULL)
	{
		fprintf(stderr,"[!] writeToFile: Measurements table is Null\n");
		return -1;
	}else{
		
		//displays the measurements by index and ADC value
		//Index is the first column
		FILE *write_ptr;

		write_ptr = fopen("../data/plot.dat","w");  // w for write
		if (write_ptr!=NULL)
		{
			//displays the measurements as Voltage = f(time)
			//time is in seconds at the first column
			for (size_t i = 0; i <(BLOCK_SIZE/2)*numberOfBlocks; i++)
			{	
				// Conversion  to real Voltage is made: 4095 max value 3.3 max voltage
				fprintf(write_ptr,"%li %0.6f\n",i, (measures[i]/4095.0)*3.3);
			}
			
			fclose(write_ptr);
			return 0;
		}
		else{
			fprintf(stderr,"[!] Unable to open a file for writing.\n");
			return -1;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////
int writeToCSV(u_int16_t* measures, size_t numberOfBlocks){
	if (measures==NULL)
	{
		fprintf(stderr,"[!] writeToCSV: Measurements table is Null\n");
		return -1;
	}else{
		
		FILE *write_ptr;

		write_ptr = fopen("../data/data.csv","w");  // w for write
		if (write_ptr!=NULL)
		{
			fprintf(write_ptr,"sample,value\n");
			for (size_t i = 0; i <(BLOCK_SIZE/2)*numberOfBlocks; i++)
			{	
				fprintf(write_ptr,"%li %d\n",i, measures[i]);
			}
			
			fclose(write_ptr);
			return 0;
		}
		else{
			fprintf(stderr,"[!] Unable to open a file for writing.\n");
			return -1;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////
void displayMeasures(u_int16_t* measures, size_t numberOfBlocks){
	if (measures==NULL)
	{
		fprintf(stderr,"[!] displayMeasures: Measurements table is Null.\n");
	}else{
		//displays the measurements by index and ADC value
		//Index is the first column
		for (size_t i = 0; i <(BLOCK_SIZE/2)*numberOfBlocks; i++)
		{	
			u_int16_t mesure= ((measures[i+1]&0x00FF)<<8) | ( measures[i] & 0x00FF);
			printf("%ld\t%d\n",i, mesure);
		}
	}	
}

////////////////////////////////////////////////////////////////////////////////////