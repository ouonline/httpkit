CC := gcc
AR := ar

ifeq ($(debug), y)
    CFLAGS := -g
else
    CFLAGS := -O2 -DNDEBUG
endif
CFLAGS := $(CFLAGS) -Wall -Werror -Wextra -fPIC

MODULE_NAME := httpkit

INCLUDE :=
LIBS :=

OBJS := $(patsubst %.c, %.o, $(wildcard *.c)) deps/utils/str_utils.o

TARGET := lib$(MODULE_NAME).a lib$(MODULE_NAME).so

.PHONY: all clean

all: $(TARGET)

lib$(MODULE_NAME).a: $(OBJS)
	$(AR) -rc $@ $^

lib$(MODULE_NAME).so: $(OBJS)
	$(CC) -shared -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
