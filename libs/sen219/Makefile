BIN_PATH =../../bin

TARGET=$(BIN_PATH)/sen219.o
DEPENDENCIES=SEN219.c SEN219.h
CFLAG = -Wall -Werror -Wextra

# build types
_debugBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) -c SEN219.c $(CFLAG) $(CDEBUG) -D_DEBUG 

_defBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) -c SEN219.c $(CFLAG)


###################### Modify build type here
default: _debugBuild


clean:
	$(RM) $(TARGET)