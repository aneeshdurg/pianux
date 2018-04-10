all: test

test: algebraic.h test.c
	gcc test.c -o test

algebraic.h: macro_iter.h

macro_iter.h: macro_iter.pyp.h
	./py_processor.py macro_iter.pyp.h 

clean:
	rm -rf macro_iter.h test
