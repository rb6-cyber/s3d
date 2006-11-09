BIN=s3dosm
VERS=0.0.1
SOURCES=main.c http_fetcher.c http_error_codes.c osm.c avl.c object.c draw.c kismet.c nav.c tag.c
HEADERS=s3dosm.h http_fetcher.h
OBJS=$(SOURCES:.c=.o)
CFLAGS=-Wall -ggdb -pedantic `xml2-config --cflags`
LDFLAGS=-lm -ls3d  `xml2-config --libs`
CC=gcc
LD=gcc



.SUFFIXES: .c .o
	
default: 	all
all:		$(BIN)
clean:
	rm -rf $(OBJS) $(BIN)
$(BIN):    $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $(BIN)
.c.o:  $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@


