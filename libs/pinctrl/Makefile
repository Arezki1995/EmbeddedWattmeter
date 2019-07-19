BIN_PATH =../../bin

TARGET=$(BIN_PATH)/pinctrl.o
DEPENDENCIES=pinctrl.c


# build types
_debugBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) -c $(DEPENDENCIES) $(CFLAG) -D_DEBUG 

_defBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) -c $(DEPENDENCIES) $(CFLAG)


###################### Modify build type here
default: _debugBuild


clean:
	$(RM) $(TARGET)