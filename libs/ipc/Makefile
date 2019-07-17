BIN_PATH =../../bin

TARGET=$(BIN_PATH)/ipc.o
DEPENDENCIES=ipc.c ipc.h
CFLAG = -Wall -Werror -Wextra

# build types
_debugBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) -c ipc.c $(CFLAG) $(CDEBUG) -D_DEBUG 

_defBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) -c ipc.c $(CFLAG)


###################### Modify build type here
default: _debugBuild


clean:
	$(RM) $(TARGET)