SHELL:=/bin/bash

all: bin/summary

bin/summary: src/summary.c
	mkdir -p bin
	gcc $^ -o $@.tmp -lm
	mv $@.tmp $@

archive: summary.tar.gz 

summary.tar.gz: src/summary.c COPYING Makefile
	cd .. && tar -cvzf summary/summary.tar.gz summary/src/summary.c summary/COPYING summary/Makefile

clean:
	rm -rf bin/ summary.tar.gz
