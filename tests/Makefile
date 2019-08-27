CC := gcc

ifeq ($(debug), y)
    CFLAGS := -g
else
    CFLAGS := -O2 -DNDEBUG
endif
CFLAGS := $(CFLAGS) -Wall -Werror -Wextra -fPIC

INCLUDE := -I..
LIBS := ../libhttpkit.a

OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

TARGET := test

.PHONY: all clean pre-process post-clean

all: $(TARGET)

pre-process:
	$(MAKE) -C ..

post-clean:
	$(MAKE) clean -C ..

$(TARGET): $(OBJS) | pre-process
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean: | post-clean
	rm -f $(TARGET) $(OBJS)
