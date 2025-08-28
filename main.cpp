// Simple calculator 
// Written with Stroustrup's book PPP 3rd ed.

/*
	Version history:
		- 2025/07/29, init, ver. 0.1 by dnlsbv 
		- 2025/08/20, re-organisation of program, ver. 0.2 by dnlsbv
		- 2025/08/28, added Symbol_table for variables, ver. 0.3 by dnlsbv
	
	This is a simple calculator. 
	Input cin and output cout; 

	We use grammar for input (tokenization) // see grammar.h
	
	Known issues:
	[X] Using Stroustrup's PPPheaders
	(2025-08-21)	Removed PPP*.h files; added necessary functions
	[X] A bug: > (1+1)1; =1 
	(2025-07-31)	Modified expression function for better grammar implementation. 
	[X] Problem: 2 computations rise "infty loop"?
	(2025-07-31)	Modified calculate function; not only "eat" ';', 
					but also save the last token;
*/

#include "grammar.h"

int main()
try
{
	greetings();
	calculate();
}
catch (std::exception& e) {
	std::cerr << "error: " << e.what() << '\n';
	return 1;
}
catch (...) {
	std::cerr << "Unknown exception\n";
	return 2;
}
