BIN_PATH =../../bin
DATA_PATH=../../data
MODULES_PATH=../../libs
CFLAG = -Wall -Werror -Wextra
TARGET=$(BIN_PATH)/AcquisitionAPI

DEPENDENCIES=$(BIN_PATH)/network.o $(BIN_PATH)/serialUSB.o $(BIN_PATH)/ipc.o $(BIN_PATH)/main.o


###################### Modify build type here
# build types

_debugBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) $(DEPENDENCIES) $(CFLAG) $(CDEBUG) -D_DEBUG
	$(RM) $(BIN_PATH)/*.o

_defBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) $(DEPENDENCIES) $(CFLAG)
	$(RM) $(BIN_PATH)/*.o

###################### Modify build type here
default: $(TARGET)


$(TARGET):_debugBuild


$(BIN_PATH)/main.o: main.c
	$(CC) -o $@ -c main.c $(CFLAG)

$(BIN_PATH)/serialUSB.o:$(MODULES_PATH)/usb/serialUSB.c
	cd $(MODULES_PATH)/usb && make

$(BIN_PATH)/network.o:$(MODULES_PATH)/network/network.c
	cd $(MODULES_PATH)/network && make

$(BIN_PATH)/ipc.o:$(MODULES_PATH)/ipc/ipc.c
	cd $(MODULES_PATH)/ipc && make

clean:
	$(RM) $(TARGET) $(BIN_PATH)/main.o
	cd $(MODULES_PATH)/usb && make clean
	cd $(MODULES_PATH)/network && make clean
	cd $(MODULES_PATH)/ipc && make clean