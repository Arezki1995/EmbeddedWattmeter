#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>           // execl, fork
#include <sys/types.h>        // pid_t
#include <sys/wait.h>


#include "../libs/usb/serialUSB.h"
#include "../libs/network/network.h"


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
///////////////////////////////////////////////////////////////////////////////////

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