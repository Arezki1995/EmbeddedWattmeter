#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "../../libs/ipc/ipc.h"

// GLOBALS
FILE *   gnuplotPipe;
int      GrapherBox_ID;

// PLOTTING SCRIPT XD
char * plotScript = "set terminal x11\n"
                    "set size noratio\n"
                    "stats '../data/plot.dat' using 1 nooutput\n"
                    "set label 1 gprintf(\"Max : %g\", STATS_max)  at 10, STATS_max+STATS_max*0.05\n"
                    "set label 2 gprintf(\"Moy : %g\", STATS_mean) at 10, STATS_max+STATS_max*0.10\n"
                    "set label 3 gprintf(\"Min : %g\", STATS_min)  at 10, STATS_max+STATS_max*0.15\n" 
                    "set grid\n"
                    "set yrange [0:STATS_max+STATS_max*0.25]\n"
                    "set ylabel \"Current (A)\"\n"
                    "set xlabel \"Samples\"\n"
                    "set title \"Current Evolution\"\n"
                    "set linetype 1 lc rgb 'blue' lw 0.5 pt 1\n"
                    "plot  '../data/plot.dat' with lines title 'graph'\n"
                    ;

////////////////////////////////////////////////////////////////////////////////////////////////
void signal_handler(int signal_number) {
    // Exit cleanup code here
	if(signal_number==SIGINT){
        printf("\nCleaning...\n");
        deleteMsgBox(GrapherBox_ID);
        pclose(gnuplotPipe);
		exit(0);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
void processMessage(GRAPHER_MSG* msg_ptr){
    switch (msg_ptr->GrapherCommand)
    {
    case GR_PLOT:
        fprintf(gnuplotPipe, "%s", plotScript);
        sendMessageToBox(GRAPHER_BOX, GrapherBox_ID, FROM_GRAPHER, GR_ACK, msg_ptr);
        break;


    default:
        sendMessageToBox(GRAPHER_BOX, GrapherBox_ID, FROM_GRAPHER, GR_ERROR, msg_ptr);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{   
    
    // Opens an interface that one can use to send commands as if they were typing into the
    // gnuplot command line.  "The -persistent" keeps the plot open even after C program terminates.
    signal(SIGINT, signal_handler);
    gnuplotPipe = popen ("gnuplot -geometry 500x300 -persist", "w");
    
    if(gnuplotPipe<0){
        fprintf(stderr,"[!] Unable to open a pipe to gnuplot ! Verify that it is correctly installed\n");
    }
    
    int status= EnableIPC_MSGBOX(&GrapherBox_ID, GRAPHER_BOX_KEY );
    if (status)
    {
        fprintf(stderr, "Grapher message queue could not be enabled.\n");
    }
            
    printf("Grapher: listening for orders...\n");
    
    GRAPHER_MSG* msg_ptr=listenForMessage(GRAPHER_BOX, GrapherBox_ID, TO_GRAPHER,0);    
    if(msg_ptr!=NULL) 
    {   
        printf("**** Welcome to GRAPHER ! ****\n");
        printf("Command to execute: %d\n",msg_ptr->GrapherCommand);
        processMessage(msg_ptr);
    }
    
    free(msg_ptr);
    pclose(gnuplotPipe);
    exit(0);
    return 0;
}
