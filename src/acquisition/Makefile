BIN_PATH =../../bin
DATA_PATH=../../data
MODULES_PATH=../../libs
CFLAG = -Wall -Werror -Wextra
TARGET=$(BIN_PATH)/AcquisitionAPI

DEPENDENCIES=$(BIN_PATH)/pinctrl.o $(BIN_PATH)/network.o $(BIN_PATH)/serialUSB.o $(BIN_PATH)/ipc.o $(BIN_PATH)/sen219.o $(BIN_PATH)/main.o


###################### Modify build type here
# build types

_debugBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) $(DEPENDENCIES) $(CFLAG) $(CDEBUG) -lpthread -D_DEBUG
	$(RM) $(BIN_PATH)/*.o

_defBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) $(DEPENDENCIES) $(CFLAG) -lpthread
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

$(BIN_PATH)/pinctrl.o:$(MODULES_PATH)/pinctrl/pinctrl.c
	cd $(MODULES_PATH)/pinctrl && make

$(BIN_PATH)/sen219.o:$(MODULES_PATH)/sen219/SEN219.c
	cd $(MODULES_PATH)/sen219 && make

clean:
	$(RM) $(TARGET) $(BIN_PATH)/main.o
	cd $(MODULES_PATH)/usb && make clean
	cd $(MODULES_PATH)/network && make clean
	cd $(MODULES_PATH)/ipc && make clean
	cd $(MODULES_PATH)/pinctrl && make clean
	cd $(MODULES_PATH)/sen219 && make clean