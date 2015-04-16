# Makefile for the command-line application mastermind
# Consists of the two applications client and server and one testing unit
# OSUE 2015S - Task 1b
# Author: Johannes Vass
# Date: 21.03.2015

CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g -c
LFLAGS=-std=c99 -pedantic -Wall -g
DEBUG= -D_ENDEBUG
BUILDDIR=build
VPATH = src

all: client server own_test

client: $(BUILDDIR)/client.o
	$(CC) $(LFLAGS) -o $(BUILDDIR)/$@ $^

server: $(BUILDDIR)/server.o
	$(CC) $(LFLAGS) -o $(BUILDDIR)/$@ $^

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

own_test: tests/own_test.o
	$(CC) $(LFLAGS) -o tests/$@ $^
	
tests/own_test.o: tests/own_test.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build/*
	rm -f tests/own_test.o tests/own_test
	
