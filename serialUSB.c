#include <stdlib.h>
#include <stdio.h> 
#include <fcntl.h>            // IO
#include <unistd.h>           // execl, fork
#include <sys/types.h>        // pid_t
#include <sys/wait.h>


//---This is an average sampling rate from multiple trials (experiences)
//---the duration of asynchronous reading with DMA is a hard to calculate
#define AVG_SAMPLING_RATE 666625.0

//-- To visalize
#define FOR_ONE_SEC   	2604
#define FOR_ONE_MILISEC	3


#define BLOCKS_NB   FOR_ONE_MILISEC


//--- YOU MUST NEVER CHANGE THIS
#define BLOCK_SIZE 	512


///////////////////////////////////////////////////////////////////////////////////
void childProcessJob(){
	//--------------Child-------------
	// Child process makes sure the SerialUSB port is not used by OS
	// In order to prevent the operating system from cutting the data sent by the DUE
	// Executing this command
	// stty -F /dev/ttyACM0 raw -iexten -echo -echoe -echok -echoctl -echoke -onlcr
	#ifdef DEBUG
		printf("Child: I will execute port detatch\n");
	#endif
	execl("/bin/stty","stty","-F","/dev/ttyACM0", "raw", "-iexten", "-echo", "-echoe", "-echok" ,"-echoctl" ,"-echoke", "-onlcr",NULL);	
}

////////////////////////////////////////////////////////////////////////////////////
u_int8_t* readBlocksFromUSB(){
		
		u_int8_t* mainBuffer = malloc(BLOCKS_NB*BLOCK_SIZE*sizeof(u_int8_t));
		if(mainBuffer==NULL){
			perror("Memory for mainBuffer is too BIG!"); 
			exit(-1);
		}
		// Open SerialUSB device (ttyACM0)
		// You may have to update this according to what's detected by Qarpediem Rpi 
		int fd = open("/dev/ttyACM0", O_RDONLY ); 
		if (fd ==-1) perror("Error opening device: ttyACM0");				 
		
		int bytesRead;
		for (int k = 0; k < BLOCKS_NB; k++)
		{	
			// The Idea: I have a main Buffer I can read chunk by chunk from DUE 
			// each time i read a chunk I move the storage pointer by a chunk  
			// a chunk is 512 bytes better leave this as default
			bytesRead = read(fd, mainBuffer+(k*BLOCK_SIZE), BLOCK_SIZE);
			usleep(10);
		}
		
		// Nothing to be read
		if (bytesRead == 0)
		{
			printf("Nothing to read!\n");
		}
		// Couldn't acces the file for read
		if (bytesRead < 0)
		{
			printf("Read: %d ! Troubleshoot device\n",bytesRead);
		}
		close(fd);
		return mainBuffer;
}
////////////////////////////////////////////////////////////////////////////////////
void displayMainBuffer(u_int8_t* table){
	for (size_t i = 0; i <BLOCK_SIZE*BLOCKS_NB; i++)
	{
		printf("%ld\t%d\n",i,table[i]);
	}
}
////////////////////////////////////////////////////////////////////////////////////
u_int16_t* formatToMeasures(u_int8_t* table){
	// The main buffer stores a series of bytes read for serialUSB but a displayMeasure
	// has a resolution of 12bits thus we need to combine every two successive bytes
	// to compose a single measure (with the adequate shifting)
	// this results in a table of "u_int16_t" of half the size of the MainBuffer
	u_int16_t* measures = malloc((BLOCKS_NB)*(BLOCK_SIZE/2)*sizeof(u_int16_t));
	size_t j=0;
	for (size_t i = 0; i <BLOCK_SIZE*BLOCKS_NB; i+=2)
	{	
		measures[j]= ((table[i+1]&0x00FF)<<8) | ( table[i] & 0x00FF);
		j++;
	}	
	return measures;
}
////////////////////////////////////////////////////////////////////////////////////
int writeToBinaryFile(u_int16_t* measures){
	//displays the measurements by index and ADC value
	//Index is the first column
	FILE *write_ptr;

	write_ptr = fopen("file.bin","wb");  // w for write, b for binary
	if (write_ptr!=NULL)
	{
		fwrite(measures,sizeof(u_int16_t),(BLOCKS_NB)*(BLOCK_SIZE/2),write_ptr); // write 10 bytes from our buffer
		fclose(write_ptr);
		return 0;
	}
	else{
		return -1;
	}
}

////////////////////////////////////////////////////////////////////////////////////
void displayMeasures(u_int16_t* measures){
	//displays the measurements by index and ADC value
	//Index is the first column
	for (size_t i = 0; i <(BLOCK_SIZE/2)*BLOCKS_NB; i++)
	{	
		u_int16_t mesure= ((measures[i+1]&0x00FF)<<8) | ( measures[i] & 0x00FF);
		printf("%ld\t%d\n",i, mesure);
	}	
}

////////////////////////////////////////////////////////////////////////////////////
void displayMeasuresWithUnits(u_int16_t* measures){
	//displays the measurements as Voltage = f(time)
	//time is in seconds at the first column
	for (size_t i = 0; i <(BLOCK_SIZE/2)*BLOCKS_NB; i++)
	{	
		printf("%f\t%0.6f\n",i/AVG_SAMPLING_RATE, (measures[i]/4095.0)*5);
	}	
}
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
int main() 
{
	pid_t childPID;
	int   childExitStatus;
	if( (childPID= fork())==0 ) childProcessJob();
	else
	{
		//--------------Father-------------
		#ifdef DEBUG
			printf("Father: I will wait for child to finish\n");
		#endif

		// Wait Child to finish	
		pid_t rc_pid = waitpid( childPID, &childExitStatus, 0);
		
		// Verify if everything was ok with the child Job
		if (rc_pid > 0)
		{
			if (WIFEXITED(childExitStatus)) {
				#ifdef DEBUG
					printf("Child exited with status: %d\n",WEXITSTATUS(childExitStatus));
				#endif
				// Exit the program if the Device is not correctly detatched from OS
				if(WEXITSTATUS(childExitStatus)) {
					exit(1);
				}
			}
		}
		///////////////////////////// 
		#ifdef DEBUG
			printf("Father: Child completed\n");	
		#endif

		u_int8_t* 	table 	 = readBlocksFromUSB();
		u_int16_t* 	measures = formatToMeasures(table);
		free(table);

		displayMeasuresWithUnits(measures);

		#ifdef _PLOT
			displayMeasuresWithUnits(measures);
		#endif
		
		#ifdef _BINARY
			if(writeToBinaryFile(measures)) printf("BAD FILE WRITE !\n");	
		#endif
		
		free(measures);
	}	
return 0; 
} 
