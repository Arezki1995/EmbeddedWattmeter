#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>           // execl, fork
#include <sys/types.h>        // pid_t
#include <sys/wait.h>
#include <fcntl.h> 
#include <time.h>

#include "../libs/usb/serialUSB.h"
#include "../libs/network/network.h"

///////////////////////////////////////////////////////////////////////////////////
char* timeStampe(){
   //HEADER DATE
		char* buf= malloc(200*sizeof(char));
        time_t now = time(0);
        struct tm tm = *gmtime(&now);
        strftime(buf, 200, "%H:%M:%S", &tm);
        return buf;
}


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
	exit(-1);	
}

///////////////////////////////////////////////////////////////////////////////////
int main() 
{
	pid_t childPID;
	int   childExitStatus;
	size_t   size =BLOCKS_NB*BLOCK_SIZE;
	char  message[size];
	char* host ="192.168.43.8";
	char* port ="9000";

	// Program sends data continously if all is OK or reinitializes every 5 sec otherwise
	// Until it detects the device this allows to make the program plug and play
	while (1)
	{
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
							fprintf(stderr,"[!] Main: ttyACM0 not ready or undetected\n");
						}
					}
				}

				#ifdef DEBUG
					printf("Father: Child completed and device caught\n");	
				#endif

				int fd = open("/dev/ttyACM0", O_RDONLY ); 
				///////////////////////////// 
				while (1)
				{
						u_int8_t* 	table 	 = readBlocksFromUSB(fd);
						if(table==NULL) break;
						
						#ifdef _PLOT
						u_int16_t* measures = formatToMeasures(table);
						displayMeasuresWithUnits(measures);
						#endif
						
						#ifdef _BINARY
						u_int16_t* measures = formatToMeasures(table);
						if(writeToBinaryFile(measures)) printf("BAD FILE WRITE !\n");	
						#endif
						
						/*
						//TO BE SENT LATER
						char* stamp=timeStampe();
						printf("%s\n", stamp);
						free(stamp);
						*/
					
						//TO BE THREADED
						int link=sendTCPmsg(host, port,table, size);
						free(table);

						
						if (!link) {
							fprintf(stderr,"[!] network: bad link.\n");
							break;
						}
					
				}
				close(fd);	
			}
	
	fprintf(stderr,"[!] Program reinitialization...\n\n");
	sleep(5);	
	}

	return 0; 
} 	