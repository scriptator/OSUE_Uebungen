# OSUE 2015S - Task 2
# Author: Johannes Vass
# Date: 23.04.2015

CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g -c
LFLAGS=-std=c99 -pedantic -Wall -g
DEBUG= -D_ENDEBUG
BUILDDIR=build
VPATH = src

all: mygzip

mygzip: $(BUILDDIR)/mygzip.o
	$(CC) $(LFLAGS) -o $(BUILDDIR)/$@ $^

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build/*	
