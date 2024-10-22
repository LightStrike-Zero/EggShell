CC = gcc

CFLAGS = -Wall -g

TARGET = simple_shell


all: $(TARGET)

$(TARGET): simple_shell.o token.o history.o formatting.o
	$(CC) $(CFLAGS) -o $(TARGET) simple_shell.o token.o history.o formatting.o



# $(SERVER_TARGET): server.o
#	$(CC) $(CFLAGS) -o $(SERVER_TARGET) server.o


simple_shell.o: simple_shell.c simple_shell.h token.h history.h formatting.h
	$(CC) $(CFLAGS) -c simple_shell.c

token.o: token.c token.h
	$(CC) $(CFLAGS) -c token.c

history.o: history.c history.h
	$(CC) $(CFLAGS) -c history.c

formatting.o: formatting.c formatting.h
	$(CC) $(CFLAGS) -c formatting.c

#server.o: server.c
#	$(CC) $(CFLAGS) -c server.c

clean:
	rm -f $(TARGET)*.o

