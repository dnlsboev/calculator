#include "grammar.h"

//------------------------------------------------------------------------------

//-------- auxilary --------

// Narrowing from Stroustrup's PPP_support
// like a narrow from GSL (without overflow checking)
struct narrowing_error : public std::exception
{ const char* what() const noexcept override { return "narrowing_error"; } };

template <class T, class U>
constexpr T narrow_cast(U&& u) noexcept
{ return static_cast<T>(std::forward<U>(u)); }

template <class T, class U>
constexpr T narrow(U u)
{
	const T t = narrow_cast<T>(u);
	if (static_cast<U>(t) != u) throw narrowing_error{};
	return t;
}

double pow(const double a, const int p)
{
	if (a == 0 && p < 0) 
	{
		throw std::runtime_error("pow(): divide by zero"); 
		return 0;
	}

	if (a == 0 && p > 0) return 0;
	
	if (a == 0 && p == 0) return 1;

	if (p < 0) return pow(1/a, -p);

	double res = 1; 
	for (int i = 0; i < p; ++i)
		res *= a;
	
	return res;
}

double factorial(int n) 
{
	if ( n < 0 )
		throw std::runtime_error("factorial() is defined only for non negative integers");
	
	double res = 1;
	for (int i = 1; i <= n; ++i) 
		res *= i; 
	
	if (res < 1)	// simple overflow check
		throw std::runtime_error("factorial(): overflow");

	return res;
}

void clean_up_mess(Token_stream& ts) {
	ts.ignore(print);
}

//------------------------------------------------------------------------------

//-------- grammar implementation --------

double statement(Token_stream &ts, Symbol_table& sym_tbl)
{
	Token t = ts.get();

	switch (t.kind)
	{
	case let:	// "let" is read
		return declaration(ts, sym_tbl, true);

	case const_tok:
		return declaration(ts, sym_tbl, false);
	
	default:
		ts.putback(t);
		return expression(ts, sym_tbl);
	}
}

double declaration(Token_stream &ts, Symbol_table& sym_tbl, bool mut_fl)
{
	Token t1 = ts.get(); 
	if (t1.kind != name)
		throw std::runtime_error("declaration(): name expected");

	Token t2 = ts.get(); 
	if (t2.kind != '=')
		throw std::runtime_error("declaration: \"=\" expected in declaration of " + t1.name);
	
	double val = expression(ts, sym_tbl);
	sym_tbl.define(t1.name, val, mut_fl);
	return val; 
}

double expression(Token_stream& ts, Symbol_table& sym_tbl)
/*
	Handle with "+" and "-"
*/
{
	double left = term(ts, sym_tbl); 

	Token t = ts.get();

	while (true) 
		switch(t.kind) 
		{
		case '+':
			left += term(ts, sym_tbl);
			t = ts.get();
			break;
		
		case '-':
			left -= term(ts, sym_tbl); 
			t = ts.get();
			break; 
		
		// this cases removes situation: (expression) number, expression (expression) and etc
		case number: case '(': case '{': 
		case '=': case sqroot: case power: case name:
			throw std::runtime_error("expression(): operand expected");
			break;
		
		default:
			ts.putback(t);
			return left; 
		}
}

double term(Token_stream& ts, Symbol_table& sym_tbl)
/*
	Handle "*", "-" and "%"
*/
{
	double left = primary(ts, sym_tbl); 
	Token t = ts.get();

	while (true) 
		switch (t.kind)
		{
		case '*':
			left *= primary(ts, sym_tbl); 
			t = ts.get(); 
			break;
		
		case '/':
		{
			double d = primary(ts, sym_tbl); 
			if (d == 0) 
				throw std::runtime_error("term(), /: divide by zero");
			
			left /= d; 
			t = ts.get();
			break;
		}
		
		case '%':
		{
			int i1 = narrow<int>(left);
			int i2 = narrow<int>(primary(ts, sym_tbl));
			if (i2 == 0) 
				throw std::runtime_error("term(), %: divide by zero");
			
			left = i1 % i2; 
			t = ts.get();
			break; 
		}
		
		default:
			ts.putback(t);
			return left; 
		}
}

double primary(Token_stream& ts, Symbol_table& sym_tbl)
/*
	Handle with Names, assignment, "!", and "()", "{}", square root and power functions
*/
{
	Token t = ts.get();

	switch (t.kind) 
	{
	case '(':	// handle "(" Expression ")"
	{
		double d = expression(ts, sym_tbl);
		t = ts.get();
		if (t.kind != ')') 
			throw std::runtime_error("primary(): ')' expected");
		
		t = ts.get();
		if (t.kind == '!')	// check for "(" Expression ")!"
			return factorial(narrow<int>(d));
		else {
			ts.putback(t);
			return d;
		}
	}
	
	case '{': 	// handle "{" Expression "}"
	{
		double d = expression(ts, sym_tbl);
		t = ts.get(); 
		if (t.kind != '}') 
			throw std::runtime_error("primary(): '}' expected");
		
		t = ts.get();
		if (t.kind == '!') //	factorial check 
			return factorial(narrow<int>(d));
		else{
			ts.putback(t);
			return d;
		}
	}

	case number:
	{
		double d = t.value;
		
		t = ts.get(); 
		if (t.kind == '!')
			return factorial(narrow<int>(d));
		else {
			ts.putback(t);
			return d;
		}
	}

	case '-': return -primary(ts, sym_tbl);
	
	case '+': return primary(ts, sym_tbl);

	case name:
	{
		std::string var = t.name; 
		t = ts.get(); 
		if (t.kind == '=') { // assignment
			double val = expression(ts, sym_tbl);
			sym_tbl.set(var, val);
			return val;
		}
		else 
			ts.putback(t);

		if (!sym_tbl.is_declared(var))
			throw std::runtime_error("primary(): undeclared variable " + var);
		
		return sym_tbl.get(var);
	}
	
	case sqroot:
	{
		t = ts.get();
		if (t.kind != '(') 
			throw std::runtime_error("primary(), sqrt: '(' expected"); // check sqrt "(" Expression ")"
		ts.putback(t);

		double d = expression(ts, sym_tbl);
		if (d < 0) 
			throw std::runtime_error("primary(), sqrt: square root from negative number");
		return sqrt(d);
	}

	case power:
	{
		t = ts.get();
		if (t.kind != '(') 
			throw std::runtime_error("primary(), pow: '(' expected"); // check pow "(" ... ")"

		double d1 = expression(ts, sym_tbl); // reading expression before ','
		t = ts.get();
		if (t.kind != ',') 
			throw std::runtime_error("primary(), pow: ',' expected");
		
		double d2 = expression(ts, sym_tbl);
		int p = narrow<int>(d2); 

		t = ts.get();
		if (t.kind != ')')
			throw std::runtime_error("primary(), pow: ')' expected"); // check pow "(" ... ")"
		
		return pow(d1, p);
	}

	default:
		throw std::runtime_error("primary expected");
		return -1;
	}
}

//------------------------------------------------------------------------------

//-------- main calculator functions --------

void greetings()
{
	std::cout << "\t\tWelcome to our simple calculator!\n"
		<< "Write '"<< helpkey << "' for help and " 
		<< "'" << quitkey << "' for exit\n"
		<< "Enter your expression using floating-point number "
		<< "and press '" << print << "' to print results.\n";
}

void get_help()
{
	std::cout << "\t\tHelp\n\n"
		<< "Available operators: '+', '-', '*', '/'.\n"
		<< "Available parenthesess '(', ')' and '{', '}'.\n\n"
		<< "Available functions:\n" 
		<< "\t- factorial '!' ONLY for INTEGERS;\n"
		<< "\t- modulus '%' ONLY for INTEGERS;\n"
		<< "\t- power 'pow(x,y)' where y is ONLY INTEGER;\n"
		<< "\t- 'sqrt' is a square root.\n\n"
		<< "You can use variables:\n"
		<< "\t 'let NAME = EXPRESSION' is a declaration of variable NAME,\n"
		<< "\t 'const NAME = EXPRESSION' is a declaration of a constant NAME,\n"
		<< "\t you can assign new values via 'NAME = EXPRESSION'\n"
		<< "\t NAME must begin with a alphabetic char, "
		<< "NAME consist of alphabetic char, underscore or digit.\n"
		<< "\t we have predefined names 'e' and 'pi'.\n\n"
		<< "Press '" << print << "' to print result, "
		<< "'" << helpkey << "' to get help, and "
		<< "'" << quitkey << "' to exit.\n"
		<< "Please enter expression using floating-point number:\n";
}

void calculate() 
{
	// TODO: need a buffer for output
	// for input: 1+1; 2+2; 
	// output: = 2 \n >=4 \n > ||| must be = 2 \n = 4 \n > ...
	
	Token_stream ts;
	Symbol_table sym_tbl; 
	// predefined list
	sym_tbl.define("pi", 3.1415926535, false);
	sym_tbl.define("e", 2.7182818284, false); 
	
	while (std::cin)
	try {
		std::cout << prompt; 

		Token t = ts.get();
		while (t.kind == print) t = ts.get();	// "eating" print symbols

		if (t.kind == quit) return;			// for quit

		if (t.kind == help) {get_help(); continue;}

		ts.putback(t);
		
		double output = statement(ts, sym_tbl);
		std::cout << result << output << '\n';
	}
	catch (std::exception& e) {
		std::cerr << "error: " << e.what() << '\n';
		clean_up_mess(ts);
	}
}

//------------------------------------------------------------------------------

//-------- variables --------

void Variable::set_value(double val)
{
	if (!mutable_flag)
	{
		throw std::runtime_error("can't change constant " + name);
		return;
	}
	value = val; 
}

double Symbol_table::get(const std::string& var) const 
{
	for (const Variable& v : var_tbl) 
		if (v.get_name() == var)
			return v.get_value();
	throw std::runtime_error(var + " not found in var_tbl");
}

void Symbol_table::set(const std::string& var, double val)
{
	for (Variable& v : var_tbl)
		if (v.get_name() == var) {
			v.set_value(val); 
			return;
		}
	throw std::runtime_error(var + " not found in var_tbl");
}

bool Symbol_table::is_declared(const std::string& var) const
{
	for (const Variable& v : var_tbl) 
		if (v.get_name() == var)
			return true; 
	return false;
}

bool Symbol_table::is_prohibited(const std::string& var) const 
{
	for (const std::string& s : prohibited_names)
		if (var == s) return true; 
	return false; 
}

void Symbol_table::define(const std::string& var, double val, bool mut_fl)
{
	if (is_declared(var))
		throw std::runtime_error("define_name(): " + var + " declared twice");
	if (is_prohibited(var))
		throw std::runtime_error("define_name(): " + var + " is reserved name");
	
	var_tbl.push_back(Variable{var, val, mut_fl});
}