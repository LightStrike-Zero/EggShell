CC = gcc

CFLAGS = -Wall -g

TARGET = simple_shell

all: $(TARGET)

$(TARGET): simple_shell.o
	$(CC) $(CFLAGS) -o $(TARGET) simple_shell.o

simple_shell.o: simple_shell.c
	$(CC) $(CFLAGS) -c simple_shell.c

clean:
	rm -f $(TARGET) *.o

