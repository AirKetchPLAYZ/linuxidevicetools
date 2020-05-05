OBJS	= main.o
SOURCE	= main.cpp
OUT	= linuxidevicetools
CC	 = g++
FLAGS	 = -g -c -Wall
LFLAGS	 = -L/usr/local/lib -limobiledevice -lirecovery

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS) 


clean:
	rm -f $(OUT)

run: $(OUT)
	./$(OUT)