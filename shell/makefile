CC = gcc

CFLAGS = -Wall -g

TARGET = egg_shell

all: $(TARGET)

$(TARGET): main.o signals.o command.o token.o history.o builtins.o terminal.o protocol.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o signals.o command.o token.o history.o builtins.o terminal.o protocol.o

main.o: main.c definitions.h command.h token.h history.h builtins.h terminal.h signals.h
	$(CC) $(CFLAGS) -c main.c

signals.o: signals.c signals.h definitions.h
	$(CC) $(CFLAGS) -c signals.c

command.o: command.c command.h definitions.h
	$(CC) $(CFLAGS) -c command.c

token.o: token.c token.h definitions.h
	$(CC) $(CFLAGS) -c token.c

history.o: history.c history.h definitions.h
	$(CC) $(CFLAGS) -c history.c

builtins.o: builtins.c builtins.h definitions.h ../protocol.h
	$(CC) $(CFLAGS) -c builtins.c

terminal.o: terminal.c terminal.h definitions.h
	$(CC) $(CFLAGS) -c terminal.c

protocol.o: ../protocol.c ../protocol.h
	$(CC) $(CFLAGS) -c ../protocol.c

clean:
	rm -f $(TARGET) *.o
