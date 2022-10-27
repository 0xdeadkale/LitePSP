#Cflags used for compiling.
CFLAGS += -g -I. -Wall -Werror -std=c18
#LDFLAGS 
LDFLAGS += -L.
#LDLIBS libraries to link to the project.
LDLIBS += -lpthread


LIB1 = src/hash_table.c
LIB2 = src/file_util.c

BIN = src/file-finder.c build/hash_table.a build/file_util.a

all: setup lib file-finder

#Creates all needed directories/files for testing.
.PHONY: setup
setup:
	@mkdir -p bin build doc header src test

#lib will generate the libraries in the build folder.
.PHONY: lib
lib:
	@gcc $(CFLAGS) -fpic -shared -o build/hash_table.a $(LIB1)
	@gcc $(CFLAGS) -fpic -shared -o build/file_util.a $(LIB2)

#file-finder will generate file-finder binary in bin.
.PHONY: file-finder
file-finder: lib
	gcc $(CFLAGS) $(LDFLAGS) -o bin/file-finder $(BIN) $(LDLIBS) -lm

#valgrind will generate file-finder binary in bin, and run valgrind with test args.
.PHONY: valgrind 
valgrind: lib
	@gcc $(CFLAGS) $(LDFLAGS) -o bin/file-finder $(BIN) $(LDLIBS) -lm
	valgrind --leak-check=full bin/file-finder test/ aaa bbb ccc ddd

#clean will delete all build artifacts and binaries.
.PHONY: clean
clean:
	@$(RM) -rf bin/* build/*

#clean_all will delete all build artifacts, binaries, and test directories/files.
.PHONY: clean_all
clean_all:
	@$(RM) -rf test/* bin/* build/* 