# Makefile for the command-line application mastermind
# Consists of the two applications client and server
# OSUE 2015S - Task 1b
# Author: Johannes Vass
# Date: 21.03.2015

CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g -c
LFLAGS=-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g
BUILDDIR=build
VPATH = src

all: client server

client: $(BUILDDIR)/client.o
	$(CC) $(LFLAGS) -o $(BUILDDIR)/$@ $^

server: $(BUILDDIR)/server.o
	$(CC) $(LFLAGS) -o $(BUILDDIR)/$@ $^

$(BUILDDIR)/%.o: %.c %.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build/*
	
