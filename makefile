cxml: lexer.c
	mkdir -p build
	g++ -O3 lexer.c -o ./build/cxml.exe

clean:
	rm -rf build