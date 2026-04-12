CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = abyss_walker.exe
SRCS = main.c map.c enemy.c config.c save.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	del /Q $(TARGET) 2>nul || exit 0

.PHONY: all clean
