CC=gcc
OSUEFLAGS=-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g -c
BUILDDIR=build/
VPATH = src/

all: mysort

mysort: main.o
	$(CC) $(OSUEFLAGS) -o $(BUILDDIR)$@ $^

main.o: main.c main.h
	$(CC) $(OSUEFLAGS) -o $(BUILDDIR)$@ $(VPATH)main.c

clean:
	rm -rf build/*
	