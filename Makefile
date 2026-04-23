CC = gcc
CFLAGS = -Wall -Werror -ansi -pedantic -Iinclude
SRCS = src/main.c src/map.c src/enemy.c src/config.c src/save.c

ifeq ($(OS),Windows_NT)
SHELL = cmd.exe
TARGET = abyss_walker.exe
CLEAN_CMD = del /Q abyss_walker.exe 2>nul
else
TARGET = abyss_walker
CLEAN_CMD = rm -f abyss_walker abyss_walker.exe
endif

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	-$(CLEAN_CMD)

.PHONY: all clean
