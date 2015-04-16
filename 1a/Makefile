# Makefile for the command-line application mysort
# OSUE 2015S - Task 1a
# Author: Johannes Vass
# Date: 20.03.2015

CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g -c
LFLAGS=-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g
BUILDDIR=build
VPATH = src

all: mysort

mysort: $(BUILDDIR)/main.o $(BUILDDIR)/bufferedFileRead.o
	$(CC) $(LFLAGS) -o $(BUILDDIR)/$@ $^

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build/*
	
