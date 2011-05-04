SHELL:=/bin/bash

all: bin/summary

src/%.o: src/%.c
	gcc -g -Wall -c $<
	mv $(notdir $*.o) $@

bin/summary: src/summary.c src/dStruct.o
	mkdir -p $(dir $@)
	gcc $^ -o $@.tmp -Wall -lm -ggdb
	mv $@.tmp $@

archive: summary.tar.gz 

summary.tar.gz: src/summary.c COPYING Makefile README
	cd .. && tar -cvzf summary/summary.tar.gz summary/src/summary.c summary/COPYING summary/Makefile summary/README

clean:
	rm -rf bin/ summary.tar.gz
