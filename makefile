# Define the C compiler and flags
CC = gcc
CFLAGS = -g -Wall

# Define the name of the executable
TARGET = myshell

# List the source files
SRCS = myshell.c

# Automatically generate object file names from source files
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Implicit rule for compiling .c to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)