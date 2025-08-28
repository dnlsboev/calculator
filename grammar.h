/*
	This is grammar for working with calculator input

//------------------------------------------------------------------------------

	Grammar for input: 

	Statement:
		Print
		Quit
		Help
		Declaration 
		Expression
	
	Declaration:
		"let" Name "=" Expression
		"let const" Name "=" Expression
	
	Name:	// name can't be started with underscore
		[a..zA..Z][a..z _ A..Z 0..9]

	Expression: 
		Term "+" Expression
		Term "-" Expression

	Term:
		Primary
		Primary "*" Term 
		Primary "/" Term (Term != 0)
		Primary "%" Term (Term != 0 && Primary and Term only integers)

	Primary: 
		Number
		"-" Primary
		"+" Primary
		"(" Expression ")"
		"{" Expression "}"
		sqrt "(" Expression ")" // Expression must be nonnegative
		pow "(" Expression1 "," Expression2 ")" // Expression2 must be integer
		Primary "!" // factorial defined only for nonnegative int Primary
		Name
		Name "=" Expression // Assignment
							// so it can be "x = y = z = 10;"

	Number:
		float-point numbers 
*/

#ifndef GRAMMAR_H
#define GRAMMAR_H

#include"token_stream.h"
#include<string>
#include<vector>

//------------------------------------------------------------------------------

//-------- auxilary --------

double factorial(int n);
double pow(const double a, const int p);
void clean_up_mess(Token_stream& ts); // for error handling

constexpr std::string prompt = "> ";
constexpr std::string result = "= ";

//------------------------------------------------------------------------------

//-------- variables --------

class Variable {
public:
	Variable(std::string var, double val, bool mut_fl = true)
		: name {var}, value {val}, mutable_flag {mut_fl} {};
	
	std::string get_name() const {return name;} 
	double get_value() const {return value;}
	bool is_mutable() const {return mutable_flag;}
	void set_value(double val);

private:
	std::string name; 
	double value; 
	bool mutable_flag; 
};

class Symbol_table {
public:
	double get(const std::string& var) const; 
	void set(const std::string& var, double val);
	bool is_declared(const std::string& var) const;
	void define(const std::string& var, double val, bool mut_fl = true);
	bool is_prohibited(const std::string& var) const;

private: 
	std::vector<Variable> var_tbl; 
	const std::vector<std::string> prohibited_names {"const", "pow", "sqrt"};
};

//------------------------------------------------------------------------------

//-------- main calculator functions --------

void greetings(); 
void get_help();
void calculate();

//------------------------------------------------------------------------------

//-------- grammar implementation --------

double statement(Token_stream& ts, Symbol_table& sym_tbl);
double declaration(Token_stream &ts, Symbol_table& sym_tbl, bool mut_fl);
double expression(Token_stream& ts, Symbol_table& sym_tbl);
double term(Token_stream& ts, Symbol_table& sym_tbl);
double primary(Token_stream& ts, Symbol_table& sym_tbl);

//------------------------------------------------------------------------------

#endif