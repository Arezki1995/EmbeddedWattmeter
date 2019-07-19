#include <stdio.h>
#include <sys/ipc.h>
#include <sys/signal.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h> 
#include <stdlib.h>
#include "../../libs/ipc/ipc.h"

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

///////////// GLOBAL VARIABLES ///////////
	int 		Config_MsgBoxID;

	CONFIG_MSG_TYPE 	type=EXT_TO_API;
	API_COMMAND  		APICommand;
	API_EXPORT  		APIExport;	
	ACQUISITION_PT  	point;				
	SAMPLING_RATE 		SamplingRate;		
	int 				numberOfBlocks;		
	char 				device[32];			
	char 				fileName[32];		
	char 				host[32];			
	char 				port[8];	

///////////////////////////////////////////////////////////////////////
void printUsage(){
	printf("\nUsage: ./interface -c <COMMAND> -e <EXPORT> -m <MSR_POINT> -s <SAMPLING_RATE> -b <NB_BLOCKS> -d <DEVICE> -f <FILE_NAME> -h <SERVER_IP> -p <SERVER_PORT> \n"); 
}
///////////////////////////////////////////////////////////////////////
void setSamplingRate(char* optarg){
	if(strcmp(optarg, "SR_666K")==0) 	{SamplingRate=SR_666K; 	return;}
	if(strcmp(optarg, "SR_280K")==0) 	{SamplingRate=SR_280K; 	return;}
	if(strcmp(optarg, "SR_125K")==0)	{SamplingRate=SR_125K; 	return;}
	if(strcmp(optarg, "SR_60K")==0)		{SamplingRate=SR_60K; 	return;}

	printUsage();
	exit(1);
}

///////////////////////////////////////////////////////////////////////
void setAcquisitionPoint(char* optarg){
	if(strcmp(optarg, "I0")==0) 		{point=I0; 		return;}
	if(strcmp(optarg, "I1")==0) 		{point=I1; 		return;}
	if(strcmp(optarg, "I2")==0) 		{point=I2; 		return;}
	if(strcmp(optarg, "I3")==0) 		{point=I3; 		return;}
	if(strcmp(optarg, "I4")==0) 		{point=I4; 		return;}
	if(strcmp(optarg, "I5")==0) 		{point=I5; 		return;}
	if(strcmp(optarg, "I6")==0) 		{point=I6; 		return;}
	if(strcmp(optarg, "I7")==0) 		{point=I7; 		return;}
	if(strcmp(optarg, "I8")==0) 		{point=I8; 		return;}
	if(strcmp(optarg, "I9")==0) 		{point=I9; 		return;}
	if(strcmp(optarg, "I10")==0) 		{point=I10; 	return;}
	if(strcmp(optarg, "I11")==0) 		{point=I11; 	return;}
	
	printUsage();
	exit(1);
}

///////////////////////////////////////////////////////////////////////
void setExportOption(char* optarg){
	if(strcmp(optarg, "CSV")==0) 		{APIExport=CSV; 	return;}
	if(strcmp(optarg, "GRAPH")==0) 		{APIExport=GRAPH; 	return;}
	if(strcmp(optarg, "NETWORK")==0)	{APIExport=NETWORK; return;}

	printUsage();
	exit(1);
}
///////////////////////////////////////////////////////////////////////
void setAPICommand(char* optarg){
	if(strcmp(optarg, "API_ACQUIRE")==0) 	{APICommand=API_ACQUIRE; 	return;}
	if(strcmp(optarg, "API_FREERUN")==0) 	{APICommand=API_FREERUN; 	return;}
	if(strcmp(optarg, "API_STOP")==0)		{APICommand=API_STOP; 		return;}
	if(strcmp(optarg, "API_SET_CONFIG")==0) {APICommand=API_SET_CONFIG; return;}
	if(strcmp(optarg, "API_GET_CONFIG")==0) {APICommand=API_GET_CONFIG; return;}
	if(strcmp(optarg, "API_ERROR")==0) 		{APICommand=API_ERROR; 		return;}
	if(strcmp(optarg, "API_QUIT")==0) 		{APICommand=API_QUIT; 		return;}

	printUsage();
	exit(1);
}

////////////////////////////////////////////////////////////////////////
/////////////////////////		MAIN		////////////////////////////
////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]){
	int opt;

	while((opt = getopt(argc, argv, "c:e:m:s:b:d:f:h:p:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'c': 	
				printf("\n\tCommand\t \t :\t %s\n",optarg);
				setAPICommand(optarg);
				break;  
            case 'e': 	
				printf("\tExport option\t :\t %s\n",optarg);
				setExportOption(optarg);
				break; 
			case 'm':	
				printf("\tMsr point\t :\t %s\n",optarg);
				setAcquisitionPoint(optarg);
				break; 
			case 's':	
				printf("\tSampling Rate\t :\t %s\n",optarg);
				setSamplingRate(optarg);
				break; 
			case 'b':	
				printf("\tNB of Blocks\t :\t %s\n",optarg);
				numberOfBlocks=atoi(optarg);
				break; 
			case 'd':	
				printf("\tDevice\t\t :\t %s\n",optarg); 
				strcpy(device,optarg);
				break; 
			case 'f':	
				printf("\tCSV File name\t :\t %s\n",optarg);
				strcpy(fileName,optarg);
				break; 
			case 'h':	
				printf("\tServer Host IP\t :\t %s\n",optarg);
				strcpy(host,optarg);
				break; 
			case 'p':   
				printf("\tServer Port\t :\t %s\n",optarg);
				strcpy(port,optarg);
				break; 
            case ':':  
                printf("option needs a value\n");  
                break;  
            case '?':  
                printf("\nUsage: ./interface -c <COMMAND> -e <EXPORT> -m <MSR_POINT> -s <SAMPLING_RATE> -b <NB_BLOCKS> -d <DEVICE> -f <FILE_NAME> -h <SERVER_IP> -p <SERVER_PORT>"
				 	   "\nRefer to Documentation for accepted values\n");
						exit(1); 
                break;  
        }  
    }  

	EnableIPC_MSGBOX( &Config_MsgBoxID  , CONFIG_BOX_KEY );
	CONFIG_MSG msg;
	
	initConfigMessage(&msg, APIExport, point, SamplingRate, numberOfBlocks, device, fileName, host, port);

	sendMessageToBox(CONFIG_BOX, Config_MsgBoxID, EXT_TO_API, APICommand, &msg);

	printf(">Request Sent !\n");

	
	return 0;
}