#include "token_stream.h"

//------------------------------------------------------------------------------

//-------- auxilary --------

std::string tolower(const std::string& s) 
{	
	std::string res;
	for (const char c : s)
		res += tolower(c);
	return res;
}

//------------------------------------------------------------------------------

void Token_stream::putback(Token t)
{
	if (full) 
		throw std::runtime_error("putback() into a full buffer");
	
	buffer = t;		// copy t to buffer
	full = true;	// buffer is now full
}

Token Token_stream::get()
{
	if (full) {
		full = false;
		return buffer;
	}

	char ch;
	std::cin >> ch;

	switch (ch) {
	case print:
	case '(':
	case ')':
	case '{':
	case '}':
	case '+':
	case '-':
	case '*':
	case '/':
	case '!':
	case '%':
	case '=':
	case ',':					// for the pow function
		return Token{ch};		// let each character represent itself

	case '.': 
	case '0': case '1': case '2': case '3': case '4': 
	case '5': case '6': case '7': case '8': case '9':
	{
		std::cin.putback(ch);
		
		double val;
		std::cin >> val;				// read a floating-point number
		return Token{number, val};
	}
	
	default:
		if (isalpha(ch)) {		// checking let or name; 
		std::string s; 
		s += ch; 
		
		while (std::cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_'))
			s += ch; 
		
		std::cin.putback(ch);
		
		if (s == declkey)
			return Token{let};

		if (s == sqrootkey)
			return Token{sqroot};
		
		if (s == powerkey)
			return Token{power};

		if (s == constkey)
			return Token{const_tok};

		if (tolower(s) == quitkey)
			return Token{quit};
		
		if (tolower(s) == helpkey)
			return Token{help};
		
		return Token{name, s};
		}
		else {
			throw std::runtime_error("get(): Bad token");
			return Token(0);
		}
	}
}

void Token_stream::ignore(char ch) // ignore input until ch 
{
	if (full && buffer.kind == ch) {
		full = false; 
		return; 
	}
	full = false; 

	char c = 0; 
	while (std::cin >> c)
		if (c == ch) return;
}

//------------------------------------------------------------------------------