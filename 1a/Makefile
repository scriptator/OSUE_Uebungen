# Makefile for the command-line application mysort
# OSUE 2015S - Task 1a
# Author: Johannes Vass

CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g -c
LFLAGS=-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g
BUILDDIR=build
VPATH = src

all: mysort

mysort: $(BUILDDIR)/main.o
	$(CC) $(LFLAGS) -o $(BUILDDIR)/$@ $^

$(BUILDDIR)/%.o: %.c %.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build/*
	
