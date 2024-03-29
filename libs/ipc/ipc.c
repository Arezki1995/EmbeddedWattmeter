#include <stdio.h>
#include <sys/ipc.h>
#include <sys/signal.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include "ipc.h"

int EnableIPC_MSGBOX(int * idFDM_ptr, int FDM_KEY){
	if(idFDM_ptr!=NULL){
		if( ((*idFDM_ptr)=msgget(FDM_KEY, IPC_CREAT|0666))==-1 ){
			fprintf(stderr,"[!] IPC: could not create IPC MSG BOX\n");
			return -1;
		}
	return 0;
	}
	return -1;
}

////////////////////////////////
void deleteMsgBox(int MsgBoxID){
	if (msgctl(MsgBoxID, IPC_RMID, NULL) == -1) {
		fprintf(stderr, "Queue could not be deleted.\n");
	}
}


////////////////////////////////
int sendMessageToBox(BOX_SELECT box, int idFDM, int type, int command, void* msg_ptr){
	if(msg_ptr==NULL) return -1;

	if(box==CONFIG_BOX){
		// OTHER PARAMS HAVE TO BE INITIALIZED OUTSIDE
		((CONFIG_MSG*) msg_ptr)->type		= type;
		((CONFIG_MSG*) msg_ptr)->APICommand	= command;
		
		int state1=msgsnd(idFDM, (CONFIG_MSG*) msg_ptr, sizeof(CONFIG_MSG), 0);
		if(state1<0){
			perror("IPC:");
			fprintf(stderr, "[!] IPC: Error sending message.\n");
		} 
	}
	else if(box==GRAPHER_BOX)
	{
		((GRAPHER_MSG*) msg_ptr)->type			 = type;
		((GRAPHER_MSG*) msg_ptr)->GrapherCommand = command;
		int state2=msgsnd( idFDM, (GRAPHER_MSG*) msg_ptr, sizeof(GRAPHER_MSG), 0);
			if(state2<0){
			fprintf(stderr, "[!] IPC: Error sending message.\n");
		} 
	}else
	{
		return -1;
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////
// returns the message type it receives the user must cast it to the expected type
void* listenForMessage(BOX_SELECT box, int idFDM, int type, int flag){
	
	if(box==CONFIG_BOX){
		CONFIG_MSG* msg_ptr 	 = malloc(sizeof( (*msg_ptr)));
		msgrcv(idFDM, msg_ptr, sizeof(*msg_ptr), type, flag);
		return msg_ptr;
	}
	else if(box==GRAPHER_BOX){
		GRAPHER_MSG* msg_ptr = malloc(sizeof( (*msg_ptr)));
		msgrcv(idFDM, msg_ptr, sizeof(*msg_ptr), type, flag);
		return msg_ptr;
	}else
	{
		return NULL;
	}
}
////////////////////////////////////////////////////////////////////
//config message initialiser
void initConfigMessage(
			CONFIG_MSG*     msg_ptr,		
			API_EXPORT 		APIExport,
			ACQUISITION_PT  point,
			SAMPLING_RATE 	SamplingRate,
			int 			numberOfBlocks,
			char 			fileName[32],
			char 			host[32],
			char 			port[8]
			){

	msg_ptr->APIExport=APIExport;
	msg_ptr->point=point;
	msg_ptr->SamplingRate=SamplingRate;
	msg_ptr->numberOfBlocks=numberOfBlocks;

	strcpy(msg_ptr->fileName,fileName);
	strcpy(msg_ptr->host,host);
	strcpy(msg_ptr->port,port);
}