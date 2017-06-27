# Modify this line to change the compiler (eg, for ic instead of gcc)
CC = gcc
WINCC= x86_64-w64-mingw32-gcc



#compiler flags 
CFLAGS = -g -Wall

SRCS = src/*.c

EXEC = autocarve

autocarve_make = src/*.c

ifeq ($(OS), Windows_NT)
windows:
	mkdir bin
	$(CC) $(autocarve_make) $(CFLAGS) -o bin/$(EXEC).exe

clean: 
	del bin 

else
linux:
	mkdir -p bin
	$(CC) $(autocarve_make) $(CFLAGS) -o bin/$(EXEC).sh 
windows:
	mkdir -p bin
	$(CC) $(autocarve_make) $(CFLAGS) -o bin/$(EXEC).exe

all:
	mkdir -p bin
	$(CC) $(autocarve_make) $(CFLAGS) -o bin/$(EXEC).sh 
	$(WINCC) $(autocarve_make) $(CFLAGS) -o bin/$(EXEC).exe

clean:
	rm -rf bin

endif

