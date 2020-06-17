

objects=$(patsubst %.c,%,$(wildcard *.c))

all: $(objects)
%: %.c
	gcc -o $@ $< -lSDL2 -lm
