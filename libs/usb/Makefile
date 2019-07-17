BIN_PATH =../../bin
DATA_PATH=../../data
CFLAG = -Wall -Werror -Wextra
TARGET=$(BIN_PATH)/serialUSB.o
DEPENDENCIES=serialUSB.c serialUSB.h
FILE=$(DATA_PATH)/file

# build types

_debugBuild:$(DEPENDENCIES) 
	$(CC) -o $(TARGET) -c serialUSB.c $(CFLAG) $(CDEBUG) -D_DEBUG -g

_defBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) -c serialUSB.c $(CFLAG)


###################### Modify build type here
default: _debugBuild

clean:
	$(RM) $(TARGET) $(FILE).bin