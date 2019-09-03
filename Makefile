CC := gcc
AR := ar

ifeq ($(debug), y)
    CFLAGS := -g
else
    CFLAGS := -O2 -DNDEBUG
endif
CFLAGS := $(CFLAGS) -Wall -Werror -Wextra -fPIC

ifndef DEPSDIR
    DEPSDIR := $(shell pwd)/..
endif

MODULE_NAME := httpkit

INCLUDE := -I$(DEPSDIR)
LIBS := $(DEPSDIR)/utils/libutils.a

OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

TARGET := lib$(MODULE_NAME).a lib$(MODULE_NAME).so

.PHONY: all clean pre-process post-clean

all: $(TARGET)

$(OBJS): | pre-process

pre-process:
	d=$(DEPSDIR)/utils; if ! [ -d $$d ]; then git clone https://github.com/ouonline/utils.git $$d; fi
	$(MAKE) DEPSDIR=$(DEPSDIR) -C $(DEPSDIR)/utils

post-clean:
	$(MAKE) clean DEPSDIR=$(DEPSDIR) -C $(DEPSDIR)/utils

lib$(MODULE_NAME).a: $(OBJS)
	$(AR) -rc $@ $^

lib$(MODULE_NAME).so: $(OBJS)
	$(CC) -shared -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean: | post-clean
	rm -f $(TARGET) $(OBJS)
