CC := gcc

ifeq ($(debug), y)
    CFLAGS := -g
else
    CFLAGS := -O2 -DNDEBUG
endif

CFLAGS := $(CFLAGS) -Wall -Werror -Wextra

INCLUDE := -I../utils
LIBS :=

OBJS := $(patsubst %.c, %.o, $(wildcard *.c)) str_utils.o

TARGET := http_server

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

str_utils.o: ../utils/str_utils.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
