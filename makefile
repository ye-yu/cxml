readero: reader.c
	mkdir -p build
	gcc -Wall -Wextra -pedantic --std=c17 -O3 -c reader.c -o ./build/reader.o

readerg: reader.c
	mkdir -p build
	gcc -Og -g -c reader.c -o ./build/reader.g -DDEBUG

lexero:
	mkdir -p build
	gcc -Wall -Wextra -pedantic --std=c17 -O3 -c lexer.c -o ./build/lexer.o

lexerg:
	mkdir -p build
	gcc -Og -g -c lexer.c -o ./build/lexer.g -DDEBUG

cxml: lexer.c
	mkdir -p build
	make readero
	make lexero
	gcc -Wall -Wextra -pedantic --std=c17 -O3 ./build/lexer.o ./build/reader.o -o ./build/cxml.exe

debug: lexer.c
	mkdir -p build
	make readerg
	make lexerg
	gcc -Og -g ./build/lexer.g ./build/reader.g -o ./build/cxml.debug.exe -DDEBUG

clean:
	rm -rf build/*