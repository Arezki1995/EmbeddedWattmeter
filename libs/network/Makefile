BIN_PATH =../../bin

TARGET=$(BIN_PATH)/network.o
DEPENDENCIES=network.c network.h
CFLAG = -Wall -Werror -Wextra

# build types
_debugBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) -c network.c $(CFLAG) $(CDEBUG) -D_DEBUG 

_defBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) -c network.c $(CFLAG)


###################### Modify build type here
default: _debugBuild


clean:
	$(RM) $(TARGET)