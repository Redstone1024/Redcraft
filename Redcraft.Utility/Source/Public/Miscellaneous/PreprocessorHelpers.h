#pragma once

// Turns an preprocessor token into a real string
#define PREPROCESSOR_TO_STRING(X) PREPROCESSOR_TO_STRING_INNER(X)
#define PREPROCESSOR_TO_STRING_INNER(X) #X

// Concatenates two preprocessor tokens, performing macro expansion on them first
#define PREPROCESSOR_JOIN(X, Y) PREPROCESSOR_JOIN_INNER(X, Y)
#define PREPROCESSOR_JOIN_INNER(X, Y) X##Y

// Concatenates the first two preprocessor tokens of a variadic list, after performing macro expansion on them
#define PREPROCESSOR_JOIN_FIRST(X, ...) PREPROCESSOR_JOIN_FIRST_INNER(X, __VA_ARGS__)
#define PREPROCESSOR_JOIN_FIRST_INNER(X, ...) X##__VA_ARGS__

// Expands to the second argument or the third argument if the first argument is 1 or 0 respectively
#define PREPROCESSOR_IF(Condition, X, Y) PREPROCESSOR_JOIN(PREPROCESSOR_IF_INNER_, Condition)(X, Y)
#define PREPROCESSOR_IF_INNER_1(X, Y) X
#define PREPROCESSOR_IF_INNER_0(X, Y) Y

// Expands to the parameter list of the macro - used for when you need to pass a comma-separated identifier to another macro as a single parameter
#define PREPROCESSOR_COMMA_SEPARATED(First, Second, ...) First, Second, ##__VA_ARGS__

// Expands to nothing - used as a placeholder
#define PREPROCESSOR_NOTHING

// Removes a single layer of parentheses from a macro argument if they are present - used to allow
// brackets to be optionally added when the argument contains commas, e.g.:
//
// #define DEFINE_VARIABLE(Type, Name) PREPROCESSOR_REMOVE_OPTIONAL_PARENS(Type) Name;
//
// DEFINE_VARIABLE(int, IntVar)                  // expands to: int IntVar;
// DEFINE_VARIABLE((TPair<int, float>), PairVar) // expands to: TPair<int, float> PairVar;
#define PREPROCESSOR_REMOVE_OPTIONAL_PARENS(...) PREPROCESSOR_JOIN_FIRST(PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL,PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL __VA_ARGS__)
#define PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL(...) PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL __VA_ARGS__
#define PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPLPREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL

// Creates a string that can be used to include a header in the form "Platform/Header.h", like "Windows/Platform.h"
#define COMPILED_PLATFORM_HEADER(Suffix) PREPROCESSOR_TO_STRING(PREPROCESSOR_JOIN(PLATFORM_HEADER_NAME/, Suffix))
