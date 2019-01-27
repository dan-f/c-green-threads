gtswitch.o: gtswitch.s
	gcc -c -o gtswitch.o gtswitch.s

gthr.o:	gthr.h gthr.c
	gcc -c -o gthr.o gthr.c

main: gtswitch.o gthr.o main.c
	gcc -o main main.c gtswitch.o gthr.o

all: main
