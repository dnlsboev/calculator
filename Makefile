CC = clang++ 
CFLAGS = -std=c++20 -Wall -c

all: calculator

calculator: token_stream.o grammar.o main.o 
	$(CC) token_stream.o grammar.o main.o -o main

token_stream.o: token_stream.h token_stream.cpp
	$(CC) $(CFLAGS) token_stream.cpp

grammar.o: grammar.h grammar.cpp
	$(CC) $(CFLAGS) grammar.cpp 

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp 

clear:
	rm -rf *.o main