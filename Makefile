# Compiler and linker settings
CC = g++-13
CFLAGS = -std=c++2b -Wall -g
LDLIBS = -L/usr/local/lib -L/usr/local/lib64 -lglfw3 -lGLEW -lGL -ldl -lpthread -Wl,-rpath=/usr/local/lib64
INC = -I./fsb

# Source files and object files
SRCS = $(wildcard *.cc) $(wildcard *.c) $(wildcard *.c)
OBJS = $(SRCS:.cc=.o)
OBJS := $(OBJS:.c=.o)

# Build targets
all: run

run: $(OBJS)
	$(CC) $^ -o $@ $(LDLIBS) $(INC)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cc
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) run
