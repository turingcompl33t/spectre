# Makefile for spectre stuff. 

CFLAGS = -Wall -Werror -pedantic -march=native 

cachetime: cachetime.o
	gcc $(CFLAGS) $^ -o $@ 

flushreload: flushreload.o
	gcc $(CFLAGS) $^ -o $@ 

ooe: ooe.o
	gcc $(CFLAGS) $^ -o $@ 

attack: attack.o
	gcc $(CFLAGS) $^ -o $@ 

improved: improved.o
	gcc $(CFLAGS) $^ -o $@

full: full.o
	gcc $(CFLAGS) $^ -o $@  

clean:
	rm -f *.o
	rm -f *.~
	rm -f cachetime
	rm -rf cachetime.dSYM
	rm -f flushreload
	rm -rf flushreload.dSYM
	rm -f ooe 
	rm -rf ooe.dSYM 
	rm -f attack 
	rm -rf attack.dSYM 
	rm -f improved 
	rm -rf improved.dSYM
	rm -f full
	rm -rf full.dSYM

.PHONY: clean
