* Code and comments must be written in english
* Function and variable names must describe what they are/what they do (ask teammates if unsure)
* Function, Class, Structure, Enum and Namespace names must respect PascalCase
* Constants of Enum must respect PascalCase
* File and folder names must respect PascalCase
* Variable names must respect camelCase
* Variable members of Class/Structure must start with 'm\_' (m\_myVar)
* Global constants must be constexpr instead of preprocessor aliases, and their name must be in CAPS\_LOCK
* Curly braces must be placed on a different line if they create a scope, but they can be on the same line as other punctuation characters (ex: "()", ";")
* One-liner if/while/for statements are allowed, but without curly brackets (ex: if (smth) return;)
* Function One-liner must be placed in header file
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

   * nested class/struct/enum
   * constructor, destructor
   * function
   * variable
2. protected:

   * nested class/struct/enum
   * constructor
   * function
   * variable
3. private:

   * nested class/struct/enum
   * constructor
   * function
   * variable



* Line must be at most 150 characters long
* Functions must be at most 120 lines long
* Function parameters used to output data must have their name ending with "\_out" (ex: var\_out)
* Single-letter variables may only be used as iterators in for loops and for math (x, y, z)
* In cpp files, the first include statement must be the one directly associated to the file (ex: vector.cpp must first include vector.h)
* Do not typedef primitive types
* "using namespace" keyword is prohibited if namespace is from c++ standard or an external library (including LibMath)
* Only one class per header/cpp pair (can still have several nested class, Struct, Enum)
* Only one instruction per line allowed excepted break; in switch (ex: a++; b++; -> prohibited)

