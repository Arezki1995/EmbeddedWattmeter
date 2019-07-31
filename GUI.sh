#!/bin/bash

defP2="CSV"
defP3="I0"
defP4="SR_666K"
defP5="1"
defP6="default.csv"
defP7="127.0.0.1"
defP8="9000"

FillEmptyFilelds(){      
      if [[ "$P2" =~ ^(CSV|GRAPH|NETWORK)$ ]]
      then
         echo OK
      else
         P2="$defP2"
      fi

      if [ "$P3" == "" ] || [ "$P3" == " " ]
      then
         P3="$defP3"
      fi

      if [ "$P4" == "" ] || [ "$P4" == " " ]
      then
         P4="$defP4"
      fi

      if [ "$P5" == "" ] || [ "$P5" == " " ]
      then
         P5="$defP5"
      fi

      if [ "$P6" == "" ] || [ "$P6" == " " ]
      then
         P6="$defP6"
      fi

      if [ "$P7" == "" ] || [ "$P7" == " " ]
      then
         P7="$defP7"
      fi
      
      if [ "$P8" == "" ] || [ "$P8" == " " ]
      then
         P8="$defP8"
      fi

      if [ "$P9" == "" ] || [ "$P9" == " " ]
      then
         P9="$defP9"
      fi
}

cd bin

mainInterface(){
   REQUEST=$(zenity --forms --title="Acquisition API" --text="Select Request Type" \
         --add-combo="APICommand" --combo-values="API_ACQUIRE|API_FREERUN|API_STOP|API_SET_CONFIG|API_QUIT" )

   case "$REQUEST" in
         API_ACQUIRE)
                  #params are just default because they are not taken into account
                  ./Interface -c "API_ACQUIRE" -e $defP2 -m $defP3 -s $defP4 -b $defP5 -f $defP6 -h $defP7 -p $defP8
               ;;
            
         API_FREERUN)
                  ./Interface -c "API_FREERUN" -e $defP2 -m $defP3 -s $defP4 -b $defP5 -f $defP6 -h $defP7 -p $defP8
                  
                  zenity --info \
                  --title="Free Running Acquisition ..." \
                  --text "Press the button to stop the Acquisition\n(Long acquisition may overflow local storage !)" \
                  --width=350 \
                  --ok-label="STOP"

                  ./Interface -c "API_STOP" -e $defP2 -m $defP3 -s $defP4 -b $defP5 -f $defP6 -h $defP7 -p $defP8
               ;;
            
         API_STOP)
                  ./Interface -c "API_STOP" -e $defP2 -m $defP3 -s $defP4 -b $defP5 -f $defP6 -h $defP7 -p $defP8
               ;;
         API_SET_CONFIG)
                  FORM=$(zenity --forms --title="Acquisition API" --text="Set Configuration" \
                  --add-combo="APIExport"  --combo-values="CSV|GRAPH|NETWORK"  \
                  --add-combo="Measure Pt" --combo-values="I0|I1|I2|I3|I4|I5|I6|I7|I8|I9|I10|I11"  \
                  --add-combo="SamplingRate" --combo-values="SR_666K|SR_280K|SR_125K|SR_60K"  \
                  --add-entry="Number of blocks" \
                  --add-entry="Filename" \
                  --add-entry="Host" \
                  --add-entry="Port")
                  
                  echo $FORM
                  
                  if [ "$?" == "0" ]
                  then
                     P2=$(echo $FORM | cut -d'|' -f1)
                     P3=$(echo $FORM | cut -d'|' -f2)
                     P4=$(echo $FORM | cut -d'|' -f3)
                     P5=$(echo $FORM | cut -d'|' -f4)
                     P6=$(echo $FORM | cut -d'|' -f5)
                     P7=$(echo $FORM | cut -d'|' -f6)
                     P8=$(echo $FORM | cut -d'|' -f7)

                     # FILL EMPTY FIELDS WITH DEFAULT
                     FillEmptyFilelds
                  fi
                  ./Interface -c "API_SET_CONFIG" -e $P2 -m $P3 -s $P4 -b $P5 -f $P6 -h $P7 -p $P8
               ;;
         API_QUIT)
                  ./Interface -c "API_QUIT" -e $defP2 -m $defP3 -s $defP4 -b $defP5 -f $defP6 -h $defP7 -p $defP8
               ;;
            
         *)
               exit 1
   esac
}


while true; do
   mainInterface
done