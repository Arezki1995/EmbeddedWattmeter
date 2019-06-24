BIN_PATH =../../bin

TARGET=$(BIN_PATH)/network.o
DEPENDENCIES=network.c


# build types
_debugBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) -c $(DEPENDENCIES) $(CFLAG) $(CDEBUG) -D_DEBUG 

_defBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) -c $(DEPENDENCIES) $(CFLAG)


###################### Modify build type here
default: _debugBuild


clean:
	$(RM) $(TARGET)