#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pinctrl.h"


int CMD_PIN[NB_PINS]={16, 13, 20, 19, 12, 26,INTERRUPT_PIN};

////////////////////////////////////////////////////////////////////////////////
int GPIOExport(int pin)
{
		char buffer[BUFFER_MAX];
		ssize_t bytes_written;
		int trial = 0;
		int fd;

		do{
			// Try 5 times to open the file if still error exit with warning
			trial++;
			fd = open("/sys/class/gpio/export", O_WRONLY);
			if(trial > 5) {
				fprintf(stderr, "[!] GPIO: Failed to open export for writing! \n");
				return(-1);
			}
			usleep(10);
		}
		while (-1 == fd);

		bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
		write(fd, buffer, bytes_written);
		close(fd);
		return(0);
}
////////////////////////////////////////////////////////////////////////////////
int GPIOUnexport(int pin)
{
		char buffer[BUFFER_MAX];
		ssize_t bytes_written;
		int fd;

		fd = open("/sys/class/gpio/unexport", O_WRONLY);
		if (-1 == fd) {
			fprintf(stderr, "[!] GPIO: Failed to open unexport for writing! \n");
			return(-1);
		}

		bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
		write(fd, buffer, bytes_written);
		close(fd);
		return(0);
}
////////////////////////////////////////////////////////////////////////////////
int GPIODirection(int pin, int dir)
{
		// Construct path
		char path[DIRECTION_PATH_SIZE];
		snprintf(path, DIRECTION_PATH_SIZE, "/sys/class/gpio/gpio%d/direction", pin);
		
		static const char s_directions_str[]  = "in\0out";
		int fd;
		fd = open(path, O_WRONLY);
		if (-1 == fd) {
			fprintf(stderr, "[!] GPIO: Failed to open gpio direction for writing! (run as root)\n");
			return(-1);
		}

		if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
			fprintf(stderr, "[!] GPIO: Failed to set direction!\n");
			return(-1);
		}

		close(fd);
		return(0);
}
///////////////////////////////////////////////////////////////////////////////
int GPIORead(int pin)
{
		// Construct path
		
		char path[VALUE_PATH_SIZE];
		snprintf(path, VALUE_PATH_SIZE, "/sys/class/gpio/gpio%d/value", pin);

		char value_str[3];
		int fd;

		fd = open(path, O_RDONLY);
		if (-1 == fd) {
			fprintf(stderr, "Failed to open gpio value for reading!\n");
			return(-1);
		}

		if (-1 == read(fd, value_str, 3)) {
			fprintf(stderr, "Failed to read value!\n");
			return(-1);
		}

		close(fd);

		return(atoi(value_str));
}
/////////////////////////////////////////////////////////////////////////////
int GPIOWrite(int pin, int value)
{
	static const char s_values_str[] = "01";

	// Construct path
	char path[VALUE_PATH_SIZE];
	snprintf(path, VALUE_PATH_SIZE, "/sys/class/gpio/gpio%d/value", pin);

	int fd;
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "[!] GPIO: Failed to open gpio value for writing!\n");
		return(-1);
	}

	if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
		fprintf(stderr, "Failed to write value!\n");
		return(-1);
	}

	close(fd);
	return(0);
}
/////////////////////////////////////////////////////////////////////////////
int EnableCommandPins(){
	// Enable GPIO pins
	int i;
	for( i=0 ;i<NB_PINS;i++){
		if (-1 == GPIOExport( CMD_PIN[i] ) )  return(1);
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
int SetCommandPinsDirection(){
	// Set GPIO directions
	int i;
	for( i=0; i<NB_PINS;i++){
		if ( -1 == GPIODirection( CMD_PIN[i], OUT) ) return(1); 
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////////
int writeCommand(int cmd){
	int i;
	for(i=0; i<NB_PINS-1; i++){
		if (-1 == GPIOWrite( CMD_PIN[i], _GET_VALUE(cmd,i))) return(3);
	}
	
	// Create falling edge to trigger DUE interrupt to read the bus
	if (-1 == GPIOWrite( INTERRUPT_PIN, HIGH)) return(3);
	usleep(5);
	if (-1 == GPIOWrite( INTERRUPT_PIN, LOW)) return(3);
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
int DisableCommandPins(){
	// Disable GPIO is
	int i;
	for( i=0; i<NB_PINS; i++){
		if (-1 == GPIOUnexport(CMD_PIN[i]))  return(4);
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////