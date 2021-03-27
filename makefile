readero: reader.c
	mkdir -p build
	gcc -O3 -c reader.c -o ./build/reader.o

readerg: reader.c
	mkdir -p build
	gcc -g -c reader.c -o ./build/reader.g -DDEBUG

lexero:
	mkdir -p build
	gcc -O3 -c lexer.c -o ./build/lexer.o

lexerg:
	mkdir -p build
	gcc -g -c lexer.c -o ./build/lexer.g -DDEBUG

cxml: lexer.c
	mkdir -p build
	make readero
	make lexero
	gcc -O3 ./build/lexer.o ./build/reader.o -o ./build/cxml.exe

debug: lexer.c
	mkdir -p build
	make readerg
	make lexerg
	gcc -g ./build/lexer.g ./build/reader.g -o ./build/cxml.debug.exe -DDEBUG

clean:
	rm -rf build/*