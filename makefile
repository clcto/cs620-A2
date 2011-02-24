# vim:noet

CC=gcc
CFLAGS=-Wall -c
LDFLAGS=-L/usr/lib
SOURCEDIR=.
OBJECTDIR=.
SOURCES:=$(wildcard $(SOURCEDIR)/*.c)
OBJECTS=$(SOURCES:$(SOURCEDIR)%.c=$(OBJECTDIR)%.o)
SHELL = /bin/sh
NAME=contents

.DEFAULT_GOAL := $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJECTDIR)/%.o: $(SOURCEDIR)/%.cpp
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -rf $(OBJECTS) $(NAME)
