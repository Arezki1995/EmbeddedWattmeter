#include <stdlib.h>
#include <stdio.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <string.h>

#include "network.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// TCP CLIENT : CONNEXION ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int connexionServeurTCP(char *hote,char *service){
    struct addrinfo precisions,*resultat,*origine;
    int statut;
    int s;

    /* Creation de l'adresse de socket */
    memset(&precisions,0,sizeof precisions);
    precisions.ai_family=AF_UNSPEC;
    precisions.ai_socktype=SOCK_STREAM;
    statut=getaddrinfo(hote,service,&precisions,&origine);

    if(statut<0){ perror("connexionServeur.getaddrinfo"); exit(EXIT_FAILURE); }

    struct addrinfo *p;
    for(p=origine,resultat=origine;p!=NULL;p=p->ai_next)
        if(p->ai_family==AF_INET6){ resultat=p; break; }

    /* Creation d'une socket */
    s=socket(resultat->ai_family,resultat->ai_socktype,resultat->ai_protocol);
    if(s<0){ perror("connexionServeur.socket"); exit(EXIT_FAILURE); }

    /* Connection de la socket a l'hote */
    if(connect(s,resultat->ai_addr,resultat->ai_addrlen)<0) return -1;

    /* Liberation de la structure d'informations */
    freeaddrinfo(origine);

    return s;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// TCP CLIENT : Message ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
int sendTCPmsg(char* machine, char* port,char* message){
    int s;
    /* Connection au serveur */
    s=connexionServeurTCP(machine,port);
    if(s<0){ fprintf(stderr,"Erreur de connexion au serveur\n"); exit(EXIT_FAILURE); }

    write(s,message,FRAME_SIZE);

    /* On termine la connexion */
    shutdown(s,SHUT_RDWR);

    return 0;
}