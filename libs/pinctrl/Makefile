BIN_PATH =./

TARGET=$(BIN_PATH)/pinctrl
DEPENDENCIES=pinctrl.c pinctrl.h


# build types
_debugBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) $(DEPENDENCIES) $(CFLAG) $(CDEBUG) -D_DEBUG 

_defBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) $(DEPENDENCIES) $(CFLAG)


###################### Modify build type here
default: _debugBuild


clean:
	$(RM) $(TARGET)