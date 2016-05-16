CC = clang
CFLAGS = -g
#DEBUG = -DBINARYDEBUG

SRCS = lib/linkedlist.o main.c lib/talloc.o lib/tokenizer.o lib/parser.o interpreter.c
HDRS = linkedlist.h value.h talloc.h tokenizer.h parser.h interpreter.h
OBJS = $(SRCS:.c=.o)

interpreter: $(OBJS)
	$(CC) -rdynamic $(CFLAGS) $^  -o $@

%.o : %.c $(HDRS)
	$(CC)  $(CFLAGS) $(DEBUG) -c $<  -o $@

clean:
	rm *.o
	rm interpreter

