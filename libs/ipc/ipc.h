
#ifndef _IPC
#define _IPC
	
	// Keys of the different used queues 
	#define CONFIG_BOX_KEY   1995
	#define GRAPHER_BOX_KEY	 2019


////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////     CONFIG_BOX     ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////	

// EXT_TO_API   : Category of messages comming from an external program to the acquisition API
// API_TO_EXT   : Category of messages going from the acquisition API to an external program
	typedef enum  { 
					EXT_TO_API=1,
					API_TO_EXT
					} 
	CONFIG_MSG_TYPE;


// List of Message contents that represent commands to the grapher
	typedef enum  {	
					API_ACQUIRE=1,
					API_FREERUN,
					API_STOP,
					API_SET_CONFIG,
					API_GET_CONFIG,
					API_ACK,
					API_ERROR,
					API_QUIT
					} API_COMMAND;
// List of possible output types
	typedef enum  {
					CSV=1,
					GRAPH,
					NETWORK
					} 
	API_EXPORT;


// Possible sampling rates
	typedef enum  {
					SR_666K=666660,
					SR_280K=280000,
					SR_125K=125000,
					SR_60K =60000
					} 
	SAMPLING_RATE;


	
// Possible acquisition points
	typedef enum  {
					I0=1, I1, I2, I3, I4, I5, I6, I7, I8, I9, I10, I11
					} 
	ACQUISITION_PT;
	

// Message Struct for messages between an external program and acquisition API
	typedef struct {
					CONFIG_MSG_TYPE 	type;
					API_COMMAND  	APICommand;
					API_EXPORT  	APIExport;			// default csv
					ACQUISITION_PT  point;				// default A0
					SAMPLING_RATE 	SamplingRate;		// default SR_666K
					int 			numberOfBlocks;		// default 2604 i.e 1 Sec at def Sampling rate
					char 			fileName[32];		// default data.csv
					char 			host[32];			// default localhost
					char 			port[8];			// default 9000
					} 
	CONFIG_MSG;

// helper function to initialise config messages
void initConfigMessage(
			CONFIG_MSG*     msg_ptr,		
			API_EXPORT 		APIExport,
			ACQUISITION_PT  point,
			SAMPLING_RATE 	SamplingRate,
			int 			numberOfBlocks,
			char 			fileName[32],
			char 			host[32],
			char 			port[8]
			);
////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////    GRAPHER_BOX    ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
	
// FROM_GRAPHER : Category of messages going from Grapher to acquisition API
// TO_GRAPHER   : Category of messages going from acquisition API to GRAPHER
	typedef enum  { 
					FROM_GRAPHER=1,
					TO_GRAPHER
					} 
	GRAPHER_MSG_TYPE ;

// List of Message contents that represent commands to the grapher
	typedef enum  {	
					GR_PLOT=1,
					GR_ACK,
					GR_ERROR
					} 
	GRAPHER_COMMAND; 


// Message Struct for messages between the acquisition API program and the gnuplot based Grapher
	typedef struct {
					GRAPHER_MSG_TYPE type;
					GRAPHER_COMMAND  GrapherCommand;
					char 		 	 payload[128];
	} 
	GRAPHER_MSG;
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// Message Box Selector
	typedef enum  {	
					CONFIG_BOX,
					GRAPHER_BOX
					}
	BOX_SELECT; 



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// Opens or Creates if not exist the IPC MSG QUEUE with a given FDM_key 
	int EnableIPC_MSGBOX(int * idFDM_ptr, int FDM_KEY);

// Depending on the box selected, it sends the according msg struct with desired type & command
// for configuration box other fields must be initialized outside the function
	int sendMessageToBox(BOX_SELECT box, int fdm_key, int type, int command, void* msg_ptr);

		

// Listen for a pending message in selected Box
	void* listenForMessage(BOX_SELECT box, int idFDM, int type, int flag);
	// Flag types
	#define BLOCKING_MODE 		0
	#define NON_BLOCKING_MODE 	04000

//Removes the Message boxes
	void deleteMsgBox(int MsgBoxID);


#endif
