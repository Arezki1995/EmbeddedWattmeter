BIN_PATH =../../bin
IPC_PATH =../../libs/ipc

EXECUTABLE=$(BIN_PATH)/Interface

DEPENDENCIES= interface.c $(BIN_PATH)/ipc.o 

###################### Modify build type here
default: _debugBuild

# build types
_debugBuild:$(DEPENDENCIES)
	$(CC) -o $(EXECUTABLE) $(DEPENDENCIES) $(CFLAG) $(CDEBUG) -D_DEBUG
	$(RM) $(BIN_PATH)/ipc.o  

_defBuild:$(DEPENDENCIES)
	$(CC) -o $(EXECUTABLE) $(DEPENDENCIES) $(CFLAG)
	$(RM) $(BIN_PATH)/ipc.o 

# compile ipc library
$(BIN_PATH)/ipc.o: $(IPC_PATH)/ipc.c $(IPC_PATH)/ipc.h
	cd $(IPC_PATH) && make

$(EXECUTABLE): interface.c $(BIN_PATH)/ipc.o
	$(CC) -o $(TARGET) interface.c  $(BIN_PATH)/ipc.o  $(CFLAG) $(CDEBUG) -D_DEBUG

clean:
	$(RM) $(TARGET) $(EXECUTABLE)