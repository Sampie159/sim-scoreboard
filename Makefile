CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -g

SRCS = $(wildcard ${SRCDIR}/*.c)
SRCDIR = src
OBJS = $(patsubst ${SRCDIR}/%.c, ${OBJDIR}/%.o, ${SRCS})
OBJDIR = obj

all: sim-sb

sim-sb: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm $(OBJDIR)/*
	rm sim-sb
