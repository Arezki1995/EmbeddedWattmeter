#include <stdio.h>
#include <sys/ipc.h>
#include <sys/signal.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include "../../libs/ipc/ipc.h"

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
int 		Config_MsgBoxID;

int main(int argc, char* argv[]){
	if (argc==4)
	{
		EnableIPC_MSGBOX( &Config_MsgBoxID  , CONFIG_BOX_KEY );
		API_MSG msg;

		int type	= atoi(argv[1]);
		int command	= atoi(argv[2]);
		int blocks	= atoi(argv[3]);
		
		msg.APIExport=CSV;
		msg.point=I10;
		msg.SamplingRate=SR_280K;
		msg.numberOfBlocks=blocks;
		strcpy(msg.device,"/dev/ttyACM0");
		strcpy(msg.fileName,"myFile.csv");
		strcpy(msg.host,"localhost");
		strcpy(msg.port,"8000");



		sendMessageToBox(CONFIG_BOX, Config_MsgBoxID, type, command, &msg);
	
		printf("Message Sent\n");
	}else{
		printf("Wrong. usage: ./prog  <type>  <command>  <numberOfBlocks>\n");
	}
	return 0;
}