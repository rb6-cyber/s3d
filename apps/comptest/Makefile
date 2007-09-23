LIBS=-lXcomposite -lXdamage -lXfixes -lXrender -lX11 -lXext -ls3d  -lm 
CFLAGS= -I/usr/local/x.org/include -I../../libs3d -g -O2
LDFLAGS=-g -O2 $(LIBS)  -L../../build/libs3d
SOURCES=comptest.c
OBJS=$(SOURCES:.c=.o)
CC=gcc
LD=gcc
BIN=comptest


.SUFFIXES: .c .o
all:	comptest
comptest:	$(OBJS)
	$(LD) $(LDFLAGS) $< -o $@
.c.o:  $(HEADERS)
	   $(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf $(OBJS) $(BIN)
