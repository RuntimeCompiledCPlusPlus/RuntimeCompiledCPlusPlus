#
# Makefile for the OpenAL sample programs
#

CC = gcc
CFLAGS = -I../include -O2 -Wall
LIBS = -L$(HOME)/config/lib -lAL -lGL -lglut

TARGETS = panning xldemo

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
