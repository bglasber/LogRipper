all : main.o lex.o file_reader.o
	g++ main.o lex.o file_reader.o -o proj

main.o : main.cc lex.h
	g++ -c main.cc

lex.o : lex.cc lex.h
	g++ -c lex.cc

file_reader.o : file_reader.cc file_reader.h
	g++ -c file_reader.cc

.PHONY: clean

clean :
	rm *.o proj
