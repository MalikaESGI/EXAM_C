CC = gcc
CFLAGS = -fPIC -Wall -Wextra -ldl -shared
SRC_DIR = src
BUILD_DIR = build
TARGET = interceptor.so

SRCS = $(SRC_DIR)/interceptor.c $(SRC_DIR)/utils.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

re: clean all
