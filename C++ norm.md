* Code and comments must be written in english
* Function and variable names must describe what they are/what they do (ask teammates if unsure)
* Function, Class, Structure, Enum and Namespace names must respect PascalCase
* Constants of Enum must respect PascalCase
* File and folder names must respect PascalCase
* Variable names must respect camelCase
* Variable members of Class/Structure must start with 'm\_' (m\_myVar)
* Global constants must be constexpr instead of preprocessor aliases, and their name must be in CAPS\_LOCK
* Classes must respect rule of 3
* Curly braces must be placed on a different line if they create a scope, but they can be on the same line as other punctuation characters (ex: "()", ";")
* One-liner if/while/for statements are allowed, but without curly brackets (ex: if (smth) return;)
* Variables and operators must be separated by a space
* Every condition keyword must be separated by a space from its condition (ex: if (smth) - good; if(smth) - bad)
* ASCII values must be represented by their character form, not their digital value
* \#define names must be fully written in CAPS\_LOCK
* In header files, must use include guards instead of #pragma once

 	ex:

 	#ifndef HEADER

 	#define HEADER

 	\[code]

 	#endif

* An empty line must separate preprocessor statements, includes and functions
* Class elements should be declared in the following order:

1. public:

	* nested class
	* constructor, destructor
	* function
	* variable

2. protected:

	* constructor
	* function
	* variable

3. private:

	* constructor
	* function
	* variable


* Line must be at most 140 characters long
* Functions must be at most 70 lines long
* When breaking parameter list into multiple line, the comma and operator should stay on the first line
* Function parameters used to output data must have their name ending with "\_out" (ex: var\_out)
* Unit variables must have their unit type stated in their name (ex: deltatime\_ms)
* Single-letter variables may only be used as iterators in for loops
* In cpp files, the first include statement must be the one directly associated to the file (ex: vector.cpp must first include vector.h)
* Include statements from the same library must be grouped together
* Different groups of include statements must be separated by an empty line
* Do not typedef primitive types
* When used with a namespace, "using" keyword is prohibited if namespace is from c++ standard or an external library
* Classes must follow RAII (constructor and destructor must manage creation/destruction of resources, no separate create and destroy functions)
* Only one class per header/cpp pair (can still have several Struct, Enum)
* Only one instruction per line allowed (ex: a++; b++; -> prohibited)
* No variable assignement in if statement conditions
* Lambda functions must be 5 lines or smaller
