BIN_PATH =../../bin
DATA_PATH=../../data

TARGET=$(BIN_PATH)/serialUSB.o
DEPENDENCIES=serialUSB.c
FILE=$(DATA_PATH)/file

# build types

_debugBuild:$(DEPENDENCIES) serialUSB.h
	$(CC) -o $(TARGET) -c $(DEPENDENCIES) $(CFLAG) $(CDEBUG) -D_DEBUG -g

_defBuild:$(DEPENDENCIES) serialUSB.h
	$(CC) -o $(TARGET) -c $(DEPENDENCIES) $(CFLAG)


###################### Modify build type here
default: _debugBuild

clean:
	$(RM) $(TARGET) $(FILE).bin