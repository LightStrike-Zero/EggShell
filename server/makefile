CC = gcc

CFLAGS = -Wall -g

TARGET = server

all: $(TARGET)

$(TARGET): server.o protocol.o
	$(CC) $(CFLAGS) -o $(TARGET) server.o protocol.o

server.o: server.c server.h ../protocol.h
	$(CC) $(CFLAGS) -c server.c

protocol.o: ../protocol.c ../protocol.h
	$(CC) $(CFLAGS) -c ../protocol.c

clean:
	rm -f $(TARGET) *.o
