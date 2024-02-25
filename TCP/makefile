# makefile

TARGET_CLIENT= TCPclient.out
TARGET_SERVER= TCPserver.out
CC= gcc
CFLAGS= -ansi -g -pedantic
OBJS_C= TCPclient.o 
OBJS_S= TCPserver.o

LIST_PATH=../list

$(TARGET_CLIENT): $(OBJS_C) $(TARGET_SERVER)
	$(CC) -o $(TARGET_CLIENT) $(OBJS_C)
	
$(TARGET_SERVER): $(OBJS_S) 
	$(CC) -o $(TARGET_SERVER) $(OBJS_S) -lm -L ../struct_libtary -lgends


TCPclient.o: TCPclient.c TCPclient.h 
	$(CC) $(CFLAGS) -c TCPclient.c
	
TCPserver.o: TCPserver.c TCPserver.h 
	$(CC) $(CFLAGS) -I $(LIST_PATH) -c TCPserver.c

runc:
		./$(TARGET_CLIENT)
runs:
		./$(TARGET_SERVER)

	
clean:
	rm -f $(TARGET) $(OBJS)

