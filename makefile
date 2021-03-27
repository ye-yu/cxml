readero: reader.c
	mkdir -p build
	gcc -O3 -c reader.c -o ./build/reader.o

lexero:
	mkdir -p build
	gcc -O3 -c lexer.c -o ./build/lexer.o

cxml: lexer.c
	mkdir -p build
	make readero
	make lexero
	gcc -O3 ./build/lexer.o ./build/reader.o -o ./build/cxml.exe

clean:
	rm -rf build/*