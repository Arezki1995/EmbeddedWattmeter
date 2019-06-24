#ifndef __NETWORK
#define __NETWORK

	#define FRAME_SIZE 512
	
	
	int connexionServeurTCP(char *hote,char *service);
	int sendTCPmsg(char* machine, char* port,char* message);

#endif