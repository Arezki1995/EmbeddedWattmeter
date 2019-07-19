# EmbeddedWattmeter
DUE ADC through DMA bus with a C serialUSB reader

This project aims to provide the software necessary to drive the embedded wattmeter on the Qarpediem project at IRCICA research Laboratory.
The goal is to be able to perform high speed and real time measurements on power consumption of the various modules of the pollution  monitoring station in order to provide insentives on the way to optimize energy management in IoT plateforms. The Core project is called EECIP that stands for Energy Efficient Communications in IoT Plateforms.

## Architecture
The following code is fragmented into several modules:
- DUE_DMA: this folder contains the code running on the Arduino DUE microcontroller in order to perform high speed sampling of the input signals and transfer through serial USB using Dynamic Memory Access.
- ./libs/usb: It provides necessary routines to perform data acquisition and manipulation on the Raspberry pie that is connected to the DUE through the Native USB port.
- ./libs/network: Conatains necessary functions to perform data transfer through the network in order to transmit raw data to the server machine. 
- data: represents the offline data storage folder.
- bin: contains build artifacts and binaries.
- displayScript.py: this is a python script found at " XXX " that is used to debug and display data aquisition. It performs real time visualision.
