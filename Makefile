CC=gcc
CFLAG=-Wall -Werror -Wextra
TARGET=serialUSB
FILE=file
DEPENDENCIES=serialUSB.c


# BUILDS
default:$(DEPENDENCIES)
	$(CC) -o $(TARGET) $(DEPENDENCIES) $(CFLAG)

_plotBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) $(DEPENDENCIES) $(CFLAG) -D_PLOT

_binaryBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) $(DEPENDENCIES) $(CFLAG) -D_BINARY

_debugBuild:$(DEPENDENCIES)
	$(CC) -o $(TARGET) $(DEPENDENCIES) $(CFLAG) -D_DEBUG



# COMPILATION TARGETS
ascii:_plotBuild
	./$(TARGET) > $(FILE).data

binary:_binaryBuild
	./$(TARGET)

debug:_debugBuild
	./$(TARGET)

plot:_plotBuild
	./$(TARGET) | gnuplot -p -e 'plot "/dev/stdin" using 1:2 title "ADC measurement" with lines'
clean:
	$(RM) $(TARGET) $(FILE).bin $(FILE).data 