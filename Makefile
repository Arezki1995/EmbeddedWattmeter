CC=gcc
CFLAG=-Wall -Werror -Wextra
TARGET=serialUSB
FILE=file.data
DEPENDENCIES=serialUSB.c

default:$(DEPENDENCIES)
	$(CC) -o $(TARGET) $(DEPENDENCIES) $(CFLAG)

save:default
	./$(TARGET) > $(FILE)

plot:default
	./$(TARGET) | gnuplot -p -e 'plot "/dev/stdin" using 1:2 title "ADC measurement" with lines'
clean:
	$(RM) $(TARGET) $(FILE)