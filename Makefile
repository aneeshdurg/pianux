OBJS_ALL=parser.o piano.o
OBJS_DIR=.objs
TEST_DIR=tests

all: piano

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
	cd algebraic-c/ && make algebraic.h && cd -

piano: $(OBJS_DIR)/piano.o $(OBJS_DIR)/parser.o
	gcc $^ -g -lao -lm -ldl -o $@ 

list_test: $(TEST_DIR)
	gcc list/list_test.c -g -o $(TEST_DIR)/$@

fs:
	gcc -Wall pianux.c `pkg-config fuse --cflags --libs` -lpthread -o pianux


parser_test: $(OBJS_DIR)/parser.o | $(TEST_DIR)
	gcc $^ parser/parser_test.c -g -lao -lm -ldl -o $(TEST_DIR)/$@

pipe:
	mkfifo pipe

rundebug: pipe
	cat pipe & LOGFILE=pipe ./pianux mount

unload:
	sudo umount mount

syntaxdoc: syntax.pdf

syntax.pdf: syntax.md
	pandoc syntax.md -o syntax.pdf

testdoc: manual_testing.pdf

manual_testing.pdf: manual_testing.md
	pandoc manual_testing.md -o manual_testing.pdf

clean:
	rm -rf piano pianux .objs/ tests/ *.pdf
