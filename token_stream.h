/*
	This file provides simple Token class and Token_stream.
*/

// TODO: implement input and output to any std::istream and std::ostream

#ifndef TOKEN_STREAM_H
#define TOKEN_STREAM_H

#include<iostream>
#include<string>

//------------------------------------------------------------------------------

//-------- representations --------

constexpr char number = '8';	// representation of a number in Token
constexpr char name = 'a';		// representation of a name -//-
constexpr char let = 'L';
constexpr char sqroot = 's';	// representation of sqrt function -//-
constexpr char power = 'p';		// representation of pow function -//-
constexpr char print = ';';		// symbol of the end of the statement
constexpr char quit = 'q';		// quit 
constexpr char help = 'h';		// help
constexpr char const_tok = 'c';	// const declaration

//-------- keys --------

constexpr std::string declkey = "let";
constexpr std::string sqrootkey = "sqrt";
constexpr std::string powerkey = "pow";
constexpr std::string constkey = "const";
constexpr std::string quitkey = "quit";
constexpr std::string helpkey = "help";

//------------------------------------------------------------------------------

class Token{
public:
	char kind;
	double value;					// for numbers 
	std::string name;				// for variables

	Token(char ch): kind{ch}, value{0} {};					// special symbol 
	Token(char ch, double val): kind{ch}, value{val} {};	// number 
	Token(char ch, std::string var): kind{ch}, name{var} {};// variable
};

class Token_stream {
public:
	Token_stream(): full{false}, buffer{0} {}
	Token get();
	void putback(Token t);	// moves token to buffer
	void ignore(char ch);
private:
	bool full;		// is there a Token in the buffer?
	Token buffer;	// keeps a Token to put back
};

#endif