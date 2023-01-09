LD_FLAGS+=-lncurses
CC_FLAGS+=-O2

ifeq ($(test -d bin && echo 1 || echo 0), 1)
	ALL := bin/tetris
else
	ALL := bin bin/tetris
endif

all: $(ALL)

bin:
	mkdir -p bin

bin/tetris: src/main.c
	$(CC) $< $(CC_FLAGS) $(LD_FLAGS) -o $@
