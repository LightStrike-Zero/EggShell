CC = gcc

CFLAGS = -Wall -g

TARGET = egg_shell

all: $(TARGET)

$(TARGET): main.o signals.o command.o token.o history.o formatting.o execution.o builtins.o terminal.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o signals.o command.o token.o history.o formatting.o execution.o builtins.o terminal.o

main.o: main.c definitions.h command.h token.h history.h formatting.h execution.h builtins.h terminal.h signals.h
	$(CC) $(CFLAGS) -c main.c

signals.o: signals.c signals.h definitions.h
	$(CC) $(CFLAGS) -c signals.c

command.o: command.c command.h definitions.h
	$(CC) $(CFLAGS) -c command.c

token.o: token.c token.h definitions.h
	$(CC) $(CFLAGS) -c token.c

history.o: history.c history.h definitions.h
	$(CC) $(CFLAGS) -c history.c

formatting.o: formatting.c formatting.h definitions.h
	$(CC) $(CFLAGS) -c formatting.c

execution.o: execution.c execution.h definitions.h
	$(CC) $(CFLAGS) -c execution.c

builtins.o: builtins.c builtins.h definitions.h
	$(CC) $(CFLAGS) -c builtins.c

terminal.o: terminal.c terminal.h definitions.h
	$(CC) $(CFLAGS) -c terminal.c

clean:
	rm -f $(TARGET) *.o
