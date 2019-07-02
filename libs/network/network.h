#ifndef __NETWORK
#define __NETWORK

	
	int connexionServeurTCP(char *hote,char *service);
	int sendTCPmsg(char* machine, char* port,char* message,size_t length);

#endif