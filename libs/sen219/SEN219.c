
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>				//Needed for I2C port
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>		//Needed for I2C port

#include "SEN219.h"

//--- comment or uncomment the line below to show or not debug messages 
//#define DEBUG

int i2c_fd;
int length;
unsigned char wr_buffer[BUFF_SIZE] = {0};
unsigned char rd_buffer[BUFF_SIZE] = {0};

////////////////////////////////////////////////////////////////////////////////////
//--- The methode of calculation used to calibrate the Sensor is present in the
//--- INA219 datasheet on page 12
unsigned int calulateCalibrationValue(float maxExpectedCurrent, float shuntResistor){
		
			float currentLSB= (maxExpectedCurrent)/(0x8000);
			unsigned int value= (unsigned int)((CALIBRATION_CST)/(currentLSB*shuntResistor));
			value=value & CALIBRATION_MSK;
			#ifdef DEBUG
				printf("cal value:%d\n",value);
			#endif
			return value;
}

////////////////////////////////////////////////////////////////
// Opens the I2C interface on the Raspberry pie
int I2C_init(char* filename, int* i2c_fd, int slaveAddr){
	if ((*i2c_fd = open(filename, O_RDWR)) < 0)
	{
		printf("Failed to open the i2c bus");
		return -1;
	}

	if (ioctl(*i2c_fd, I2C_SLAVE, slaveAddr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		return -1;
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
//--- write() returns the number of bytes actually written, if it doesn't match then an error occurred (e.g. no response from the device)
int I2C_write(int i2c_fd, unsigned char *wr_buffer,int length){
	
	if (write(i2c_fd, wr_buffer, length) != length)
	{
		printf("Failed to write to the i2c bus.\n");
		return -1;
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////////
//--- read() returns the number of bytes actually read.
//--- if it doesn't match then an error occurred (ex no response from the device)
int I2C_read(int i2c_fd, unsigned char *rd_buffer, int length){
	
	if (read(i2c_fd, rd_buffer, length) != length) return -1;
	else return 0 ;
}

//////////////////////////////////////////////////////////////////////////////
//--- Allows you to get the value of a specific register on the Sensor
// 	  The parameter is the registers code (DEFINED all ABOVE)
int get(int parameter){
    wr_buffer[0] = parameter;
    length = 1;

    if( I2C_write(i2c_fd, wr_buffer, length)  != STATUS_OK) perror("I2C_write");
	
	switch(parameter) {
	case CONFIG_REG  :
		#ifdef DEBUG
			printf("Reading Configuration:\n");
		#endif
		break; 
	case SHUNT_REG  :
		#ifdef DEBUG
			printf("Reading Shunt Voltage:\n");
		#endif
		break; 
	case BUS_REG  :
		#ifdef DEBUG
			printf("Reading Bus Voltage:\n");
		#endif
		break; 
	case POWER_REG  :
		#ifdef DEBUG
			printf("Reading Power:\n");
		#endif
		break; 
	case CURRENT_REG  :
		#ifdef DEBUG
			printf("Reading Current:\n");
		#endif
		break; 
	case CALIB_REG  :
		#ifdef DEBUG
			printf("Reading Calibration:\n");
		#endif
		break; 
	default : 
		#ifdef DEBUG
			printf("Unsupported  parameter !\n");
		#endif
		return -1;
	}
	
	//----- READ 2 BYTES On I2C
	length = 2;
	if( I2C_read( i2c_fd, rd_buffer, length) != STATUS_OK ) perror("I2C_read");
	int decimal= ((rd_buffer[0] & 0xFFFF)<<8) | (rd_buffer[1] & 0xFFFF);
	
	#ifdef DEBUG
		printf("\tResult: 0x%hhx | 0x%hhx  \tdecimal: %d\n\n", rd_buffer[0],rd_buffer[1], decimal);
	#endif
	return decimal;
}

//////////////////////////////////////////////////////////////////////////////
//--- Allows you to Set the value of a specific register on the Sensor
void set(int param, int value){
    if(!(param==CALIB_REG || param==CONFIG_REG)) {
			printf("Bad Parameter!\n");
			return;
		}
		wr_buffer[0] = param;
        wr_buffer[1] = (unsigned char) ((0xFF00 & value)>>8);
	    wr_buffer[2] = (unsigned char) (0x00FF & value);
	    #ifdef DEBUG
			printf("Written ======> 0x%hhx | 0x%hhx \n\n", wr_buffer[1], wr_buffer[2]);
	    #endif
		length = 3;
        if( I2C_write(i2c_fd, wr_buffer, length)  != STATUS_OK) perror("I2C_write");
}
/////////////////////////////////////////////////////////////////////////////
//--- Gives the value in mV of the bus voltage (between IN- & GND)
int getBusVoltage(){
	int val= get(BUS_REG);
	val = (val>>3);
	val = val * 4;
	return val;
}
//////////////////////////////////////////////////////////////////////////////
void initSensor(float maxCurrent, float shuntResistor){
	set(CONFIG_REG, FAST_CONFIG);
	get(CONFIG_REG);
	set(CALIB_REG, calulateCalibrationValue(maxCurrent,shuntResistor));
	get(CALIB_REG);
}

//////////////////////////////////////////////////////////////////////////////
//--- The main function that Initializes a given SEN219 device 
//--- SensorADDR : is the I2C address of the Sensor
//--- maxCurrent : is needed for calibration this is the highest expected Current at the point of measurement. refer to datasheet.
void setVoltageSensor(int SensorADDR, float maxCurrent, float shuntResistor){
		
		//----- OPEN THE I2C BUS -----
		char *filename = (char*)"/dev/i2c-1";
		if( I2C_init(filename, &i2c_fd, SensorADDR) != STATUS_OK){
				fprintf(stderr, "[!] I2C: Error opening device at given address.\n");
		}

		//----- INITIALIZE AND CALIBRATE THE SENSOR
		initSensor(maxCurrent,shuntResistor);
}

