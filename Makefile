OBJS_ALL=parser.o piano.o
OBJS_DIR=.objs
TEST_DIR=tests

all: piano

$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)
$(TEST_DIR):
	mkdir -p $(TEST_DIR)

$(OBJS_DIR)/parser.o: algebraic parser/parser.c | $(OBJS_DIR)
	gcc -o $@ parser/parser.c -g -ldl -c 

$(OBJS_DIR)/piano.o: algebraic piano.c | $(OBJS_DIR)
	gcc -o $@ piano.c -g -ldl -c 

algebraic: 
	cd algebraic-c/ && make algebraic.h && cd -

piano: $(OBJS_DIR)/piano.o $(OBJS_DIR)/parser.o
	gcc $^ -g -lao -lm -ldl -o $@ 

parser_test: $(OBJS_DIR)/parser.o | $(TEST_DIR)
	gcc $^ parser/parser_test.c -g -lao -lm -ldl -o $(TEST_DIR)/$@

pipe:
	mkfifo pipe

rundebug: pipe
	cat pipe & LOGFILE=pipe ./pianux mount

unload:
	sudo umount mount

syntaxdoc: syntax.pdf
syntax.pdf:
	pandoc syntax.md -o syntax.pdf

clean:
	rm -rf piano pianux .objs/ tests/ pipe syntax.pdf
