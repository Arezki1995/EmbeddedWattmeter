#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "../../libs/ipc/ipc.h"

#define	SR_666K "666660"
#define	SR_280K "280000"
#define	SR_125K "125000"
#define	SR_60K "60000"

// GLOBALS
FILE *   gnuplotPipe;
int      GrapherBox_ID;

// PLOTTING SCRIPT XD
char * plotScript = "set term x11\n"
                    "set size noratio\n"
                    "set grid\n"
                    "TMARGIN = 'set tmargin at screen 0.99; set bmargin at screen 0.55'\n"
                    "BMARGIN = 'set tmargin at screen 0.55; set bmargin at screen 0.05'\n"
                    "LMARGIN = 'set lmargin at screen 0.05; set rmargin at screen 0.55'\n"
                    "RMARGIN = 'set lmargin at screen 0.05; set rmargin at screen 0.99'\n"
                    "set font ',6'\n"
                    ;

char * plotPower =  "stats '../data/plot.dat' using 1 nooutput\n"
                    "set linetype 1 lc rgb 'green' lw 0.1 pt 0.1\n"
                    "set yrange [0:STATS_max+STATS_max*0.20]\n"
                    "set label 1 sprintf(\"  Max: %d mW    Avg: %d mW    Min: %d mW\", STATS_max, STATS_mean,STATS_min) at 0,STATS_max*0.10\n"
                    "set title 'Power (mW)'\n"
                    "set autoscale xfixmax\n"
                    "unset key\n"
                    ;

char * plotVoltage= "set linetype 1 lc rgb 'red' lw 0.5 pt 0.5\n"
                    "set yrange [0:10]\n"
                    "set xlabel 'Time (ms)'\n"
                    "set title 'Voltage (V)'\n"
                    "set ytics 1\n"
                    "unset label 1\n"
                    "set autoscale xfixmax\n"
                    "unset key\n"
                    ;

char * plotCurrent= "stats '../data/plot.dat' using 3 nooutput\n"
                    "set linetype 1 lc rgb 'blue' lw 0.1 pt 0.1\n"
                    "set yrange [0:STATS_max+STATS_max*0.20]\n"
                    "set label 1 sprintf(\"  Max: %d mA    Avg: %d mA    Min: %d mA\", STATS_max, STATS_mean,STATS_min) at 0,STATS_max*0.10\n"
                    "set title 'Current (mA)'\n"
                    "set autoscale xfixmax\n"
                    "unset key\n"
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
    int freq;
    switch (msg_ptr->GrapherCommand)
    {
    case GR_PLOT:

        if(strstr(msg_ptr->payload, SR_666K) != NULL) {
            freq = 666;
        }
        else if(strstr(msg_ptr->payload, SR_280K) != NULL) {
            freq = 280;
        }
        else if(strstr(msg_ptr->payload, SR_125K) != NULL) {
            freq = 125;
        }
        else{
            freq = 60;
        }

        fprintf(gnuplotPipe, "%s", plotScript);
        fprintf(gnuplotPipe,"set multiplot layout 3,1 title 'Power Current & Voltage | Info: %s' font ',20'\n", msg_ptr->payload);

        fprintf(gnuplotPipe, "%s", plotPower);
        fprintf(gnuplotPipe, "plot  '../data/plot.dat' using ($0/%d.):1 with lines\n",freq );
        fprintf(gnuplotPipe, "%s", plotCurrent);
        fprintf(gnuplotPipe, "plot  '../data/plot.dat' using ($0/%d.):3 with lines\n",freq );
        fprintf(gnuplotPipe, "%s", plotVoltage);
        fprintf(gnuplotPipe, "plot  '../data/plot.dat' using ($0/%d.):2 with lines\n",freq );
        sleep(2);

        // I you need to know if the graph has been generated uncomment to receive an GR_ACK
        //sendMessageToBox(GRAPHER_BOX, GrapherBox_ID, FROM_GRAPHER, GR_ACK, msg_ptr);
        break;


    default:
        // I you need to know if an error uncomment to receive an GR_ACK
        // sendMessageToBox(GRAPHER_BOX, GrapherBox_ID, FROM_GRAPHER, GR_ERROR, msg_ptr);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{   
    
    // Opens an interface that one can use to send commands as if they were typing into the
    // gnuplot command line.  "The -persistent" keeps the plot open even after C program terminates.
    signal(SIGINT, signal_handler);
    gnuplotPipe = popen ("gnuplot -geometry 800x900 -persist", "w");
    
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
