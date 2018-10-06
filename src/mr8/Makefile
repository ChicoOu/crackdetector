CC=g++
CFLAGS=-c -g -Wall -fPIC
LFLAGS=-g -Wall -lopencv_highgui -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
SRCS=mr8fast.cpp
OBJS=mr8fast.o
APP=mr8fast

all:compile
	$(CC) -o $(APP) $(OBJS) $(LFLAGS)

compile:
	$(CC) $(CFLAGS) $(SRCS)

test:
	./$(APP) ../../pred_90.png

clean:
	rm -rf *.o
	rm -rf $(APP)