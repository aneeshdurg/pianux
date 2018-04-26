OBJS_ALL=parser.o piano.o
OBJS_DIR=.objs
TEST_DIR=tests

all: fs 

all_tests: parser_test list_test
	./tests/list_test
	./tests/parser_test

$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

$(OBJS_DIR)/parser.o: algebraic parser/parser.c | $(OBJS_DIR)
	gcc -o $@ parser/parser.c -g -ldl -c 

$(OBJS_DIR)/piano.o: piano.c algebraic | $(OBJS_DIR)
	gcc -o $@ piano.c -g -ldl -c 

algebraic: 
	cd data_structures/algebraic-c/ && make algebraic.h && cd -

piano: $(OBJS_DIR)/piano.o $(OBJS_DIR)/parser.o
	gcc $^ -g -lao -lm -ldl -o $@ 

list_test: $(TEST_DIR)
	gcc data_structures/list/list_test.c -g -o $(TEST_DIR)/$@

fs: pianux

pianux: piano
	gcc -Wall pianux.c `pkg-config fuse --cflags --libs` -lpthread -o pianux

fs_test: pianux_test

pianux_test: pianux | $(TEST_DIR)
	gcc -Wall pianux_test.c -o $(TEST_DIR)/$@

parser_test: $(OBJS_DIR)/parser.o | $(TEST_DIR)
	gcc $^ parser/parser_test.c -g -lao -lm -ldl -o $(TEST_DIR)/$@

run:
	./pianux mount

pipe:
	mkfifo pipe

rundebug: pipe
	cat pipe & LOGFILE=pipe ./pianux mount

example: example.c
	gcc example.c -o example

unload:
	sudo umount mount

syntaxdoc: syntax.pdf

syntax.pdf: docs/syntax.md
	pandoc docs/syntax.md -o syntax.pdf

testdoc: manual_testing.pdf

manual_testing.pdf: docs/manual_testing.md
	pandoc docs/manual_testing.md -o manual_testing.pdf

pianux_docs: pianux_docs.pdf

pianux_docs.pdf: README.md
	pandoc README.md -o pianux_docs.pdf

docs: pianux_docs syntaxdoc testdoc

clean:
	rm -rf piano pianux example pipe .objs/ tests/ *.pdf
