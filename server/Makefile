# Compiler and linker settings
CC = g++-13
CFLAGS = -std=c++2b -g
LDLIBS = -L/usr/local/lib -L/usr/local/lib64 -ldl -lpthread -lcurl -Wl,-rpath=/usr/local/lib64
INC = -I../fsb

# Source files and object files
SRCS = $(wildcard *.cc) $(wildcard */*.cc) $(wildcard *.c) $(wildcard */*.c)
OBJS = $(SRCS:.cc=.o)
OBJS := $(OBJS:.c=.o)

# Build targets
all: server

server: $(OBJS)
	$(CC) $^ -o $@ $(LDLIBS) $(INC)

%.o: %.cc
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) server
