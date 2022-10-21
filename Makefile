#Cflags used for compiling.
CFLAGS += -g -I. -Wall -Werror -std=c18
#LDFLAGS 
LDFLAGS += -L.
#LDLIBS libraries to link to the project.
LDLIBS += -lpthread


LIB1 = src/hash_table.c

BIN = src/file-finder.c build/hash_table.a

#PORT = 53673

all: setup lib file-finder

#Creates all needed directories.
setup:
	@mkdir -p bin build doc include src test
	@mkdir test/subdir_1 test/subdir2 test/subdir_3
	@touch test/subdir_1/aaa test/subdir2/bbb test/subdir_3/ccc

#lib will generate the librarys in the build folder.
lib:
	@gcc $(CFLAGS) -fpic -shared -o build/hash_table.a $(LIB1)

file-finder:
	gcc $(CFLAGS) $(LDFLAGS) -o bin/file-finder $(BIN) $(LDLIBS) -lm

#server will generate ftp server binary in bin. 
valgrind: lib
	@gcc $(CFLAGS) $(LDFLAGS) -o bin/file-finder $(BIN) $(LDLIBS) -lm
	valgrind --leak-check=full bin/file-finder test/subdir_3 aaa bbb ccc

clean:
	@$(RM) -rf bin/* build/*

clean_all:
	@$(RM) -rf bin/* build/* test/*