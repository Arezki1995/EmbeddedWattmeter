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
                    "set grid\n"
                    "set samples 2\n"
                    "set yrange [0:4]\n"
                    "set ylabel \"Voltage\"\n"
                    "set xlabel \"Samples\"\n"
                    "set title \"Current Evolution\"\n"
                    "set linetype 1 lc rgb 'green' lw 2 pt 1\n"
                    "stats '../data/plot.dat' using 2 nooutput\n"
                    "set label 1 gprintf(\"Maximum = %g\", STATS_max)  at 10, STATS_max+0.6\n"
                    "set label 2 gprintf(\"Moyenne = %g\", STATS_mean) at 10, STATS_max+0.4\n"
                    "set label 3 gprintf(\"Minimum = %g\", STATS_min)  at 10, STATS_max+0.2\n" 
                    "plot  '../data/plot.dat' u 1:2 with lines title 'graph'\n"
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
void processMessage(int message){
    switch (message)
    {
    case GR_OPEN:
        
        break;
    case GR_CLOSE:
        pclose(gnuplotPipe);
        break;
    case GR_UPDATE:
        fprintf(gnuplotPipe, "%s", "reread");
        
        break;
    case GR_SET:
                
        break;
    default:
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
    //Send Script to gnuplot
    fprintf(gnuplotPipe, "%s", plotScript); 
    
    int status= EnableIPC_MSGBOX(&GrapherBox_ID, GRAPHER_BOX_KEY );
    if (status)
    {
        fprintf(stderr, "Grapher message queue could not be enabled.\n");
    }
            
    while (1)
    {   
        printf("Grapher: listening for orders...\n");
        GRAPHER_MSG* msg_ptr=listenForMessage(GRAPHER_BOX, GRAPHER_BOX_KEY, TO_GRAPHER,0);    
        if(msg_ptr!=NULL) 
        {
            printf("Main program: %d\n",msg_ptr->GrapherCommand);
            processMessage(msg_ptr->GrapherCommand);
        }
        
        sendMessageToBox(GRAPHER_BOX, GrapherBox_ID, FROM_GRAPHER, GR_ACK, msg_ptr);
        free(msg_ptr);
    }
    
    return 0;
}
