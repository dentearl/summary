SHELL:=/bin/bash -e
export SHELLOPTS=pipefail

CC=gcc
OPTS=-Wall -Werror -Wextra -std=c99 -pedantic -g -lm

.PHONY: all clean archive
all: bin/summary

src/%.o: src/%.c src/%.h
	${CC} ${OPTS} -c $<
	mv $(notdir $*.o) $@

bin/summary: src/summary.c src/dStruct.o
	mkdir -p $(dir $@)
	${CC} $^ -o $@.tmp ${OPTS}
	mv $@.tmp $@

archive: summary.tar.gz 

summary.tar.gz: src/summary.c src/dStruct.c src/dStruct.h COPYING Makefile README.md
	tar -cvzf summary.tar.gz COPYING Makefile README.md src/summary.c src/dStruct.c src/dStruct.h 

clean:
	rm -rf bin/ src/*.o summary.tar.gz
