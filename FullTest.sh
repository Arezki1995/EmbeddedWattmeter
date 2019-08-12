#!/bin/bash

defP2="GRAPH"

defP4="SR_60K"
defP5="1"
defP6="default.csv"
defP7="127.0.0.1"
defP8="9000"


implementedSensors="I1 I2 I3 I4 I8"
NonImplementedSensors="I0 I6 I7 I9 I10 I11"

cd bin

for point in $implementedSensors
do
	echo "Acquiring $defP5 blocks on $point"
	./Interface -c "API_SET_CONFIG" -e $defP2 -m $point -s $defP4 -b $defP5 -f $defP6 -h $defP7 -p $defP8
	./Interface -c "API_ACQUIRE" -e $defP2 -m $point -s $defP4 -b $defP5 -f $defP6 -h $defP7 -p $defP8

done

