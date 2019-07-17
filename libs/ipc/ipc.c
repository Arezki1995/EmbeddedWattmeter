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
		fprintf(stderr, "Config queue could not be deleted.\n");
	}
}


////////////////////////////////
int sendMessageToBox(BOX_SELECT box, int idFDM, int type, int command, void* msg_ptr){
	if(msg_ptr==NULL) return -1;

	if(box==CONFIG_BOX){
		// OTHER PARAMS HAVE TO BE INITIALIZED OUTSIDE
		((API_MSG*) msg_ptr)->type			= type;
		((API_MSG*) msg_ptr)->APICommand	= command;
		
		msgsnd(idFDM, (API_MSG*) msg_ptr, sizeof(API_MSG), 0);
	}
	else if(box==GRAPHER_BOX)
	{
		((GRAPHER_MSG*) msg_ptr)->type			 = type;
		((GRAPHER_MSG*) msg_ptr)->GrapherCommand = command;
		msgsnd( idFDM, (API_MSG*) msg_ptr, sizeof(GRAPHER_MSG), 0);
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
		API_MSG* msg_ptr 	 = malloc(sizeof( (*msg_ptr)));
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