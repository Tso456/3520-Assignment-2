#Makefile to compile sched.c
CC = gcc
LDFLAGS = -lcrypt -lcrypto -lpthread

TARGETS = sched

all: $(TARGETS)

sched: sched.c
	$(CC) -g -o sched sched.c $(LDFLAGS)

.PHONY: clean

clean:
	@rm -f $(TARGETS) *.o core