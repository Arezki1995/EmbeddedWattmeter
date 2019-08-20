#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>           // execl, fork
#include <sys/types.h>        // pid_t
#include <sys/wait.h>
#include <fcntl.h> 
#include <string.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#include "../../libs/pinctrl/pinctrl.h"
#include "../../libs/usb/serialUSB.h"
#include "../../libs/network/network.h"
#include "../../libs/ipc/ipc.h"
#include "../../libs/sen219/SEN219.h"

/////////////////////////////////
// Display GLOBALS
// 0th indexes are place holder because these enums start from 1 to avoid problems with kernel handling zeros
// in the message struct fields. Putting zeroes in a field causes the message not to be delivered. To overcome
// this issue I decided to start all enums from a non zero value. 
char* exportString[]={" ","CSV","GRAPH","NETWORK"};
char* pointsString[]={" " ,"I0", "I1", "I2", "I3", "I4", "I5", "I6", "I7", "I8", "I9", "I10", "I11"};

////////// GLOBALS //////////////
	u_int8_t* 	table=NULL;
	u_int16_t*  I=NULL;
	int 		Config_MsgBoxID;
	int 		Grapher_MsgBoxID;
	int 		fd;  
	
	// configuration defaults
	API_EXPORT  	current_APIExport		=GRAPH;	
	ACQUISITION_PT  current_point			=I1;				
	SAMPLING_RATE 	current_samplingRate	=SR_666K;
	int 			current_NbOfBlocks		=10;		
	char 			device[32]				="/dev/ttyACM0";
	char 			fileName[32]			="data.csv";
	char 			host[32]				="127.0.0.1";
	char 			port[8]					="9000";		

	float 			AdcToVoltageCST    		= (3.3/4095);
	int 			deviceIndex				= 0;
	char* 			deviceOptions[]			= {"/dev/ttyACM0","/dev/ttyACM1"};
/// TO BE DONE

	pthread_t 		voltageThread_id; 
	float*			V;
	
	// VDR: voltage distribution ratio 
	// NB_Voltage samples = NBcurrent_samples/VDR
	// since we cannot sample voltage very fast we distribute it  
	// sampling if VDR=64 it means 1 Voltage sample for 64 current Samples
	// !!! Make sure it is a power of 2 in the range of [2:256]
	int 			VDR=64;

// ELECTRICAL SETTINGS FOR CALIBRATION

	// Values of the shunt resistors to convert current measurement. expressed in Ohms
	// first value is just a place holder
	// THE VALUES with 0.8 are the points with the BLUE current sensor
	// They have a special formula
	// I am not sure of the calibration for those because I Have No way to test that in INRIA
	// so Check them and adapt the calibration if needed but normally it is OK
	float ShuntResistors[]		= {-999,(0.8), (0.8), 0.2, 1.0, 1.0, 0.5, (0.8), (0.8), 1.0, 0.5, 0.5, 1.0};
	
	// Maximum expected currents on measurement points to calibrate Voltage measurement
	// refer to the SEN219 datasheet for more deatils
	// the values choosen here are subjective = (Max measured + some margin)
	// the precision is quite good nearly 1% error.
	// values are in mA
	// first value is just a place holder
	float MaxCurrentRatings[]	= {-999 ,600,2000,1200,200,200,600,1000,500,300,300,300,200 };

	// I2C voltage sensor addresses 
	int voltageSensorAddr[]		= {0x41, 0x44, 0x45};

///////////////////////////////////////////////////////////////////////////////////
// Returns the I2C address of the voltage sensor depending on the point of measurement
int getVoltmeterAddrForCurrentPoint(){
		switch (current_point)
		{
			case I0:  return voltageSensorAddr[0]; break;
			case I1:  return voltageSensorAddr[0]; break;
			case I2:  return voltageSensorAddr[0]; break;
			case I3:  return voltageSensorAddr[2]; break;
			case I4:  return voltageSensorAddr[2]; break;
			case I5:  return voltageSensorAddr[1]; break;
			case I6:  return voltageSensorAddr[0]; break;
			case I7:  return voltageSensorAddr[1]; break;
			case I8:  return voltageSensorAddr[1]; break;
			case I9:  return voltageSensorAddr[1]; break;
			case I10: return voltageSensorAddr[2]; break;
			case I11: return voltageSensorAddr[2]; break;
			default:
				return -1;
				break;
		}
}	
///////////////////////////////////////////////////////////////////////////////////
// a function to display current configuration parameters
void displayConfiguration(){
	printf("######################### CONFIGURATION ########################\n");
	printf("\n\tACQUISITION:\n");
	printf("\t\tdevice\t\t: \t%s\n",device);
	
	printf("\t\tMeasure point\t: \t%s\n",pointsString[current_point]);
	printf("\t\tSamplingRate\t: \t%d Sample/s\n",current_samplingRate);
	printf("\t\tNB of Blocks\t: \t%d \n",current_NbOfBlocks);
	printf("\t\texport option\t: \t%s\n",exportString[current_APIExport]);
	printf("\t\tCSV filename\t: \t%s\n",fileName);

	printf("\n\tNETWORK:\n");
	printf("\t\tserver IP\t: \t%s\n",host);
	printf("\t\tserver port\t: \t%s\n",port);
	printf("\n\tMSG QUEUES:\n");
	printf("\t\tconfig key\t:\t%d\n",CONFIG_BOX_KEY);
	printf("\t\tgrapher key\t:\t%d\n",GRAPHER_BOX_KEY);
}
///////////////////////////////////////////////////////////////////////////////////
// handling a brupt program exit 
void signal_handler(int signal_number) {
    // Exit cleanup code here
	if(signal_number==SIGINT){
		DisableCommandPins();
		deleteMsgBox(Config_MsgBoxID);
		deleteMsgBox(Grapher_MsgBoxID);
		close(fd);
		exit(0);
    }
}
///////////////////////////////////////////////////////////////////////////////////
// Generate a unix time stampe
char* timeStampe(){
	time_t current_time;
    char* c_time_string;

    /* Obtain current time. */
    current_time = time(NULL);

    if (current_time == ((time_t)-1))
    {
        fprintf(stderr, "Failure to obtain the current time.\n");
		return "noDate";
	}

    /* Convert to local time format. */
    c_time_string = ctime(&current_time);

    if (c_time_string == NULL)
    {
        fprintf(stderr, "Failure to convert the current time.\n");
        return "noDate";
    }

	return c_time_string;

}
///////////////////////////////////////////////////////////////////////////////////
// Child process makes sure the data sent through SerialUSB port is treated as raw 
// data by OS In order to prevent the operating system from cutting the data sent by the DUE
// Executing this command
// stty -F "/dev/ttyACMn" raw -iexten -echo -echoe -echok -echoctl -echoke -onlcr
void childProcessJob(){
	execl("/bin/stty","stty","-F",device, "raw", "-iexten", "-echo", "-echoe", "-echok" ,"-echoctl" ,"-echoke", "-onlcr",NULL);
	exit(-1);	
}
///////////////////////////////////////////////////////////////////////////////////
// Handles Grapher process action
void GrapherProcessJob(){
	//--------------Child: Grapher-------------
	#ifdef DEBUG
		printf("Child: I will execute port detatch\n");
	#endif
	execl("./Grapher","./Grapher",NULL);
	exit(-1);	
}
///////////////////////////////////////////////////////////////////////////////////
// sets current configuration according to input request message
// according to the export type some field are not set.
void setConfiguration(CONFIG_MSG config_msg){

	current_APIExport		=config_msg.APIExport;	
	current_point			=config_msg.point;				
	current_samplingRate	=config_msg.SamplingRate;
	current_NbOfBlocks		=config_msg.numberOfBlocks;		

	// THIS PARAMS HAVE TO BE CORRECTLY INITIALIZED IN THE EXTERNAL PROGRAM
	// USER MUST TAKE CARE OF INPUT VALIDATION

	if(current_APIExport==CSV){
		strcpy(fileName, config_msg.fileName);
	}else if(current_APIExport==NETWORK){
		strcpy(host,	 config_msg.host);
		strcpy(port, 	 config_msg.port);
	}


	/// Setup the correct Voltage acquisition sensor depending on measurement point
	setVoltageSensor(getVoltmeterAddrForCurrentPoint() , MaxCurrentRatings[current_point], ShuntResistors[current_point]);

}
///////////////////////////////////////////////////////////////////////////////////
// sends back a message with the current configuration
void getConfiguration(CONFIG_MSG* config_msg_ptr){
	if (config_msg_ptr==NULL)
	{
		fprintf(stderr,"[!] getConfiguration: Message struct pointer NULL\n");
		return;
	}
	
	config_msg_ptr->APIExport 	   = current_APIExport;	
	config_msg_ptr->point     	   = current_point;
	config_msg_ptr->SamplingRate   = current_samplingRate ;
	config_msg_ptr->numberOfBlocks = current_NbOfBlocks;		
	strcpy( config_msg_ptr->fileName  , fileName);
	strcpy( config_msg_ptr->host      , host	);
	strcpy( config_msg_ptr->port	  , port	);
}
///////////////////////////////////////////////////////////////////////////////////
// Export Acquisition into a CSV File 
// set header to Null if you dont want one for this data chunk (to use in FreeRunning
// acquisition for example in order not to write the header many times)
int writeToCSV(u_int16_t* PowerValues, size_t numberOfBlocks, char header[256]){
	if (PowerValues==NULL)
	{
		fprintf(stderr,"[!] Main, writeToCSV: Measurements table is Null\n");
		return -1;
	}else{
		
		FILE *write_ptr;
		char outputFile[48];
		outputFile[47]='\0';
		sprintf(outputFile, "../data/%s", fileName);
		write_ptr = fopen(	outputFile	,"a");  // a for append
		if (write_ptr!=NULL)
		{	
			if(header!=NULL){
				fprintf(write_ptr, header );
			}
			
			size_t i ;
			for (i = 0; i <(BLOCK_SIZE/2)*numberOfBlocks; i++)
			{	
				fprintf(write_ptr,"%d\n",PowerValues[i]);
			}
			
			fclose(write_ptr);
			return 0;
		}
		else{
			fprintf(stderr,"[!] Unable to open the file for writing.\n");
			return -1;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////
int isNotInModal(__attribute__((unused)) u_int16_t element, __attribute__((unused)) float errorTolerance){

	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	///////       MODAL CODE GOES HERE       ////////
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////

	return 1;
}
///////////////////////////////////////////////////////////////////////////////////
u_int16_t* getPower(){
	u_int16_t* PowerValues = malloc( (current_NbOfBlocks*(BLOCK_SIZE/2))*sizeof(*PowerValues) );
	int k=0;
	for(k=0; k<current_NbOfBlocks*(BLOCK_SIZE/2); k++){
		switch(current_point){
			case I0:
			case I1:
			case I6:
			case I7:
				// Calibration formula for the Blue Current Sensors
				PowerValues[k]= (u_int16_t) ((I[k] * AdcToVoltageCST) * ShuntResistors[current_point] * V[k/VDR]) ;
				break;
			default:
				// Calibration formula for the Red Current Sensors
				// Applying the formula to convert voltage measured into Current according to shunt resistor
				// then Power = Current x Voltage : note that voltage is in mV thus we have power in mW
				PowerValues[k]= (u_int16_t) ((I[k] * AdcToVoltageCST) * (1/(ShuntResistors[current_point]*10))*(V[k/VDR]))  ;
				break;
		}
	}


	return PowerValues;
}

///////////////////////////////////////////////////////////////////////////////////
// processes data export depending on the current export configuration
int  ExportAcquisition(char* startTime){
		
		// voltage and current thread synchronization
		// wait for voltage measure to finish if not done yet
		pthread_join(voltageThread_id, NULL);

		int link;
		char header[256];
		
		switch (current_APIExport)
		{
			case CSV:
				
				if(startTime!=NULL){
					char* p;
					// Remove new line from timeStamp
					for (p = startTime;( p = strchr(p, '\n')) ; ++p) { *p = ' '; }
					header[255]='\0';
					//create the header
					sprintf(header, "%s, %s, %d\n", startTime, pointsString[current_point], current_samplingRate);
				}

				I = formatRawMeasurements(table,current_NbOfBlocks);
				free(table);
				if(I==NULL) return -1;
				
				/////////////////////////////////////////
				///  Applying calibration conversion  ///
				/////////////////////////////////////////
				u_int16_t* PowerValues = getPower();
				free(I);
				free(V);
				// when no start time is given, the header is not writen to the csv !!
				if(startTime!=NULL){
					writeToCSV(PowerValues, current_NbOfBlocks, header);	
				}else
				{
					writeToCSV(PowerValues, current_NbOfBlocks, NULL);
				}
				free(PowerValues);
				break;
			

			case GRAPH:
				printf(">Exporting to Graph\n");
				I = formatRawMeasurements(table,current_NbOfBlocks);
				free(table);
				if(I==NULL) return -1;
		
				/////////////////////////////////////////
				///  Applying calibration conversion  ///
				/////////////////////////////////////////
				u_int16_t* PlotPowerValues = getPower();
				

				// Write to the plot file
				FILE *write_ptr;
				write_ptr = fopen(	"../data/plot.dat"	,"w+");
				if (write_ptr!=NULL)
				{	
					int i, k;
					float current ;
					for (i = 0; i <(BLOCK_SIZE/2)*current_NbOfBlocks; i++)
					{	
						k=(int) (i/VDR);
						current=  ( I[i] * AdcToVoltageCST *1000* (1/(ShuntResistors[current_point]*10)) );
						fprintf(write_ptr,"%d %0.3f %0.1f\n",PlotPowerValues[i], (V[k]/1000), current  );
					}
					fclose(write_ptr);
				}
				
				free(I);
				free(V);
				free(PlotPowerValues);

				pid_t GrapherPID;
				if( (GrapherPID= fork())==0 ) {
				//--------------Child: Grapher-------------
					GrapherProcessJob();
				}
				else
				{
					GRAPHER_MSG gr_msg;
					
					gr_msg.payload[127]='\0';
					char* p;
					char* timeNow= timeStampe();
					// Remove new line from timeStamp
					for (p = timeNow;( p = strchr(p, '\n')) ; ++p) { *p = ' '; }
					
					sprintf(gr_msg.payload,"input %s, %d Samples/s, %s",pointsString[current_point], current_samplingRate,timeNow);
					sendMessageToBox(GRAPHER_BOX,Grapher_MsgBoxID,TO_GRAPHER,GR_PLOT,&gr_msg);

					// we wait Until Grapher terminates
					wait(NULL);
				}

				break;
			

			case NETWORK:

				I = formatRawMeasurements(table,current_NbOfBlocks);
				free(table);
				if(I==NULL) return -1;
		
				/////////////////////////////////////////
				///  Applying calibration conversion  ///
				/////////////////////////////////////////
				u_int16_t* NetPowerValues = getPower();
				free(I);
				free(V);
				// the pointer offset that stores at each step of the loop the shift of the Block being tested relative to the first element 
				// in the NetPowerValues array
				// |------------------------->chunkShift
 				// [ ----- ][  m-2  ][  m-1  ][   m   ][ ----- ]...
				// |----------------- NetPowerValues array  ----------------->|
				int chunkShift;
				int m;
				for(m=0; m<current_NbOfBlocks; m++){

					chunkShift=(m*(BLOCK_SIZE/2));
					
					// wheather the bloc is in the modal or not
					// We use a random point on each block for validatiion 
					if(isNotInModal(NetPowerValues[chunkShift+(rand()%256)],5))
					{
						// not in modal we have to send the block
						// remember calibrated values are 16bits normally i would have sent BLOCK_SIZE/2
						// but since we need to split them into 8 bits chars this means that in order to 
						// send 256 measures I need to send 512 (chars) and BLOCK_SIZE=512.
						// [ -- ][ -- ][ -- ]...
						// [-][-][-][-][-][-]...
						link=sendTCPmsg(host, (char*)port, (char*)NetPowerValues + chunkShift, (BLOCK_SIZE));
						
						
						if (!link) 
						{
							fprintf(stderr,"[!] network: bad link.\n");
							return -1;
						}
					}
				}
				free(NetPowerValues);
				break;
			
			default:
				break;
		}
		return 0;
}
///////////////////////////////////////////////////////////////////////////////////
// Thread function that Measures voltage samples
void* voltageMeasureThread(__attribute__((unused)) void *vargp) { 
	int NBsamples =current_NbOfBlocks*(BLOCK_SIZE/(2*VDR));
	V= malloc( NBsamples * sizeof(float));
    int i=0;
	for(i=0; i<NBsamples ; i++){
		V[i]=getBusVoltage();
	}
    return NULL; 
}
///////////////////////////////////////////////////////////////////////////////////
// starts data acquisition depending on acquisitionMode
int startAcquisition(int acquisitionMode){


		// DATA ACQUISITION
		char* startTime= timeStampe();
		if(acquisitionMode==API_ACQUIRE){
			

			/////////   ACQUIRE MODE /////////

				// Launch voltage measurement thread 
				pthread_create(&voltageThread_id, NULL, voltageMeasureThread, NULL);


				// launch current acquisiton in main thread
				table 	 = readCurrentRawValues(fd,current_NbOfBlocks);
				if(table==NULL)    return -1;
				ExportAcquisition(startTime);
		}else
		{
			//////// FreeRunning MODE ////////
				if(current_APIExport==GRAPH) {
					fprintf(stderr,"[!] GRAPH export not allowed in freeRunning Mode !\n");
					return -1;
				}

				int firstLine=1;
				while (1)
				{	
					CONFIG_MSG* config_msg_ptr =(CONFIG_MSG*) listenForMessage(CONFIG_BOX, Config_MsgBoxID, EXT_TO_API, NON_BLOCKING_MODE);
					if(config_msg_ptr->APICommand==API_STOP){
						// Process stop order
						return 0;
					}

					// Launch voltage measurement thread 
					pthread_create(&voltageThread_id, NULL, voltageMeasureThread, NULL);
					
					// launch current acquisiton in main thread
					table 	 = readCurrentRawValues(fd,current_NbOfBlocks);
					if(table==NULL)    return -1;
					if(firstLine){

						ExportAcquisition(startTime);
					}else
					{

						ExportAcquisition(NULL);
					}
				}
		}
	return 0;
}
////////////////
// Sends Hardware configuration to the arduino Due Via control Bus
void ConfigureDUE(){
	// Send configurations to arduino DUE 
	// measurement point
	// The reason there is a shift of 1 in measurement point is the fact that
	// starting with 0 creates issues with the CONFIG message struct to avoid this
	// I prefered to start from 1. In the arduino Due's firmware, Though, measurement points are labeled 
	// starting from 0. this little contraint forces us to adjust the measurement point number 
	// to the equivalent on the DUE by substracting 1.
	
	// determine mux select code depending on current measurement point
	int muxSelect;
	switch(current_point){
		case I1:
		case I3:
		case I5:
			muxSelect=(MUX_I1_I3_I5<<4);
			break;

		case I2:
		case I4:
		case I7:
			muxSelect=(MUX_I2_I4_I7<<4);
			break;

		case I6:
		case I8:
		case I10:
			muxSelect=(MUX_I6_I8_I10<<4);
			break;
		
		case I0:
		case I9:
		case I11:
			muxSelect=(MUX_I0_I9_I11<<4);
			break;
		
		default:
			break;
	}
	
	int command = (muxSelect) | (current_point-1);
	writeCommand(command);
	// SamplingRate
	if(current_samplingRate==SR_666K){ writeCommand( muxSelect | ADC_MODE_666K); }
	if(current_samplingRate==SR_280K){ writeCommand( muxSelect | ADC_MODE_280K); }
	if(current_samplingRate==SR_125K){ writeCommand( muxSelect | ADC_MODE_125K); }
	if(current_samplingRate==SR_60K ){ writeCommand( muxSelect | ADC_MODE_60K ); }

}

///////////////////////////////////////////////////////////////////////////////////
/////////////////////   MAIN   ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
int main(int argc , char* argv[]) {
	
	// for random number generation
	srand(time(NULL));
	
	// getting the device from commande line if submitted or select a default otherwise
	if(argc ==2){
		strcpy(device, argv[1]);
	}else{
		strcpy(device, deviceOptions[deviceIndex]);
	}

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

				// Hardware Control pins setup
				EnableCommandPins();
				SetCommandPinsDirection();
				ConfigureDUE();
				
				if (USBLoopStatus==0){
						// Show global configuration parameters
						displayConfiguration();
						// API LOOP
						while (1)
						{
								// wait to get message, treat it, then free it
								printf("\nAPI Listening For Requests:\n");
								CONFIG_MSG* config_msg_ptr =(CONFIG_MSG*) listenForMessage(CONFIG_BOX, Config_MsgBoxID,EXT_TO_API,0);
								
								printf("Request Received :%d\n",config_msg_ptr->APICommand);
								switch (config_msg_ptr->APICommand)
								{
									case API_ACQUIRE:
										//Start acquisition
										//sendMessageToBox(CONFIG_BOX,Config_MsgBoxID,API_TO_EXT,API_ACK,config_msg_ptr);
										printf("-->API_ACQUIRE\n");
										
										// just to flush the buffer from any previous acquisition
										free(readCurrentRawValues(fd,50));
										
										if(startAcquisition(API_ACQUIRE)) USBdisconnected=1;
										break;

									case API_FREERUN:
										//Start free run acquisition
										//sendMessageToBox(CONFIG_BOX,Config_MsgBoxID,API_TO_EXT,API_ACK,config_msg_ptr);
										printf("-->API_FREERUN\n");

										// just to flush the buffer from any previous acquisition
										free(readCurrentRawValues(fd,50));

										if(startAcquisition(API_FREERUN)) USBdisconnected=1;
										break;

									case API_STOP:
										//sendMessageToBox(CONFIG_BOX,Config_MsgBoxID,API_TO_EXT,API_ACK,config_msg_ptr);
										printf("-->API_STOP: No FreeRunning Acquisition !\n");
										break;
									
									case API_SET_CONFIG:
										// Set the current configuration to message content
										//sendMessageToBox(CONFIG_BOX,Config_MsgBoxID,API_TO_EXT,API_ACK,config_msg_ptr);
										printf("-->API_SET_CONFIG\n");
										
										// Update configuration variables
										setConfiguration(*config_msg_ptr);
										ConfigureDUE();
										usleep(100);
										displayConfiguration();
										break;

									case API_GET_CONFIG:
										//Send back current configuration to External program
										//sendMessageToBox(CONFIG_BOX,Config_MsgBoxID,API_TO_EXT,API_ACK,config_msg_ptr);
										printf("-->API_GET_CONFIG\n");
										getConfiguration(config_msg_ptr);
										break;
									
									case API_QUIT:
										//Exit program
										//sendMessageToBox(CONFIG_BOX,Config_MsgBoxID,API_TO_EXT,API_ACK,config_msg_ptr);
										printf("-->API_QUIT\n");
										free(config_msg_ptr);
										close(fd);
										DisableCommandPins();
										deleteMsgBox(Config_MsgBoxID);
										deleteMsgBox(Grapher_MsgBoxID);
										exit(0);
										break;
									
									default:
										fprintf(stderr,"[!] Main: API Command not supported\n");
										sendMessageToBox(CONFIG_BOX,Config_MsgBoxID,API_TO_EXT,API_ERROR,config_msg_ptr);
										break;
								}
								
								free(config_msg_ptr);
								if (USBdisconnected) break;
						}
				}
			}
			fprintf(stderr,"[!] Verify USB cable...\n");
			deviceIndex++;
			strcpy(device, deviceOptions[(deviceIndex)%2]);
			fprintf(stderr,"[!] Testing other device ID...\n\n");
			sleep(1);
	}
	return 0; 
} 	