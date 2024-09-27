CC = gcc

CFLAGS = -Wall -g

TARGET = simple_shell

all: $(TARGET)

$(TARGET): simple_shell.o token.o history.o
	$(CC) $(CFLAGS) -o $(TARGET) simple_shell.o token.o history.o

simple_shell.o: simple_shell.c token.h history.h
	$(CC) $(CFLAGS) -c simple_shell.c

token.o: token.c token.h
	$(CC) $(CFLAGS) -c token.c

history.o: history.c history.h
	$(CC) $(CFLAGS) -c history.c

clean:
	rm -f $(TARGET) *.o

