#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>           // execl, fork
#include <sys/types.h>        // pid_t
#include <sys/wait.h>
#include <fcntl.h> 
#include <string.h>
#include <time.h>
#include <signal.h>


#include "../../libs/pinctrl/pinctrl.h"
#include "../../libs/usb/serialUSB.h"
#include "../../libs/network/network.h"
#include "../../libs/ipc/ipc.h"

/////////////////////////////////
char* exportString[]={" ","CSV","GRAPH","NETWORK"};

////////// GLOBALS //////////////
	u_int8_t* 	table=NULL;
	u_int16_t*  measures=NULL;
	int 		Config_MsgBoxID;
	int 		Grapher_MsgBoxID;
	int 		fd;  
	
	// configuration defaults
	API_EXPORT  	current_APIExport		=CSV;	
	ACQUISITION_PT  current_point			=I0;				
	SAMPLING_RATE 	current_samplingRate	=SR_666K;
	int 			current_NbOfBlocks		=1;		
	char 			device[32]				="/dev/ttyACM0";
	char 			fileName[32]			="data.csv";
	char 			host[32]				="127.0.0.1";
	char 			port[8]					="9000";		


///////////////////////////////////////////////////////////////////////////////////
void displayConfiguration(){
	printf("######################### CONFIGURATION ########################\n");
	printf("\n\tACQUISITION:\n");
	printf("\t\tdevice\t\t: \t%s\n",device);
	
	printf("\t\tMeasure point\t: \tI%d\n",current_point);
	printf("\t\tSamplingRate\t: \t%d Sample/s\n",current_samplingRate);
	printf("\t\tNB of Blocks\t: \t%d \n",current_NbOfBlocks);
	printf("\t\texport option\t: \t%s\n",exportString[current_APIExport]);
	printf("\t\tfilename\t: \t%s\n",fileName);

	printf("\n\tNETWORK:\n");
	printf("\t\tserver IP\t: \t%s\n",host);
	printf("\t\tserver port\t: \t%s\n",port);
	printf("\n\tMSG QUEUES:\n");
	printf("\t\tconfig key\t:\t%d\n",CONFIG_BOX_KEY);
	printf("\t\tgrapher key\t:\t%d\n",GRAPHER_BOX_KEY);
}
///////////////////////////////////////////////////////////////////////////////////
void signal_handler(int signal_number) {
    // Exit cleanup code here
	if(signal_number==SIGINT){
		deleteMsgBox(Config_MsgBoxID);
		close(fd);
		exit(0);
    }
}
///////////////////////////////////////////////////////////////////////////////////
char* timeStampe(){
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
	execl("/bin/stty","stty","-F",device, "raw", "-iexten", "-echo", "-echoe", "-echok" ,"-echoctl" ,"-echoke", "-onlcr",NULL);
	exit(-1);	
}
///////////////////////////////////////////////////////////////////////////////////
void setConfiguration(API_MSG config_msg){

		current_APIExport		=config_msg.APIExport;	
		current_point			=config_msg.point;				
		current_samplingRate	=config_msg.SamplingRate;
		current_NbOfBlocks		=config_msg.numberOfBlocks;		
		
	// THIS PARAMS HAVE TO BE CORRECTLY INITIALIZED IN THE EXTERNAL PROGRAM
	// Input validation is not the objective here

		strcpy(device,	 config_msg.device);

		strcpy(fileName, config_msg.fileName);

		strcpy(host,	 config_msg.host);

		strcpy(port, 	 config_msg.port);
}
///////////////////////////////////////////////////////////////////////////////////
void getConfiguration(API_MSG* config_msg_ptr){
	if (config_msg_ptr==NULL)
	{
		fprintf(stderr,"[!] getConfiguration: Message struct pointer NULL\n");
		return;
	}
	
	config_msg_ptr->APIExport 	   = current_APIExport;	
	config_msg_ptr->point     	   = current_point;
	config_msg_ptr->SamplingRate   = current_samplingRate ;
	config_msg_ptr->numberOfBlocks = current_NbOfBlocks;		
	strcpy( config_msg_ptr->device    , device	);
	strcpy( config_msg_ptr->fileName  , fileName);
	strcpy( config_msg_ptr->host      , host	);
	strcpy( config_msg_ptr->port	  , port	);
}
///////////////////////////////////////////////////////////////////////////////////
int startAcquisition(){
		int link;
		// DATA ACQUISITION
		table 	 = readCurrentRawValues(fd,current_NbOfBlocks);
		if(table==NULL)    return -1;

		switch (current_APIExport)
		{
			case CSV:
				printf("Exporting to CSV\n");
				measures = formatRawMeasurements(table,current_NbOfBlocks);
				if(measures==NULL) return -1;
				
				free(table);
				writeToCSV(measures, current_NbOfBlocks);	
				free(measures);
				break;
			
			case GRAPH:
				printf("Exporting to Graph\n");
				measures = formatRawMeasurements(table,current_NbOfBlocks);
				if(measures==NULL) return -1;
				//TO BE DONE
				free(table);
				free(measures);
				break;
			
			case NETWORK:
				//TO BE THREADED
				printf("Exporting to Network\n");
				link=sendTCPmsg(host, (char*)port, (char*)table, current_NbOfBlocks*BLOCK_SIZE);
				free(table);
				
				if (!link) {
					fprintf(stderr,"[!] network: bad link.\n");
					return -1;
				}
				break;
			
			default:
				break;
		}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////
/////////////////////   MAIN   ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
int main() {
	pid_t childPID;
	int   childExitStatus;
	signal(SIGINT, signal_handler);
	
	// DEVICE DETATCH LOOP
	// Loops Until it detects the device this allows to make the program plug and play
	while (1)
	{
			int   USBLoopStatus=0;
			int   USBdisconnected=0;
			if( (childPID= fork())==0 ) {
				//--------------Child-------------
				childProcessJob();
			}
			else
			{
				//--------------Father------------
				// Wait Child to finish	
				pid_t rc_pid = waitpid( childPID, &childExitStatus, 0);
				// Verify if everything was ok with the child Job
				if (rc_pid > 0){
					if (WIFEXITED(childExitStatus)) {
						// Exit the program if the Device was not correctly detatched from OS or unconnected
						if(WEXITSTATUS(childExitStatus)) {
							fprintf(stderr,"[!] Main: usb device not ready or undetected\n");
							USBLoopStatus=1;
						}
					}
				}			
				// Open the usb port
				fd = open(device, O_RDONLY );
				if (fd <0){
					fprintf(stderr,"[!] Main: Error opening usb device\n");
					USBLoopStatus=1;
				} 
				// get/create message boxes for configuration and graphing 
				EnableIPC_MSGBOX( &Config_MsgBoxID  , CONFIG_BOX_KEY );
				EnableIPC_MSGBOX( &Grapher_MsgBoxID , GRAPHER_BOX_KEY);

				if (USBLoopStatus==0){
						// Show global configuration parameters
						displayConfiguration();
						// API LOOP
						while (1)
						{
								// wait to get message, treat it, then free it
								printf("\nListening For Requests:\n");
								API_MSG* config_msg_ptr =(API_MSG*) listenForMessage(CONFIG_BOX, Config_MsgBoxID,EXT_TO_API,0);
								
								printf("Request Received :%d\n",config_msg_ptr->APICommand);
								switch (config_msg_ptr->APICommand)
								{
									case START:
										//Start acquisition
										printf("-->START\n");
										if(startAcquisition()) USBdisconnected=1;
										break;

									case QUIT:
										//Exit program
										printf("-->EXIT\n");
										free(config_msg_ptr);
										close(fd);
										exit(0);
										break;
									
									case SET_CONFIG:
										// Set the current configuration to message content
										printf("-->SET CONF\n");
										setConfiguration(*config_msg_ptr);
										displayConfiguration();
										break;

									case GET_CONFIG:
										//Send back current configuration to External program
										printf("-->GET CONF\n");
										getConfiguration(config_msg_ptr);
										sendMessageToBox(CONFIG_BOX,CONFIG_BOX_KEY,API_TO_EXT,ACK,config_msg_ptr);
										break;
					
									default:
										fprintf(stderr,"[!] Main: API Command not supported\n");
										break;
								}
								free(config_msg_ptr);
								if (USBdisconnected)
								{
									// Loop on the USBLoop
									break;
								}
								
						}
				}
			}
			fprintf(stderr,"[!] Verify USB cable...\n\n");
			sleep(5);
	}
	return 0; 
} 	