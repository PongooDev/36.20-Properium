// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Turns an preprocessor token into a real string (see UBT_COMPILED_PLATFORM)
 */
#define PREPROCESSOR_TO_STRING(x) PREPROCESSOR_TO_STRING_INNER(x)
#define PREPROCESSOR_TO_STRING_INNER(x) #x

/**
 * Concatenates two preprocessor tokens, performing macro expansion on them first
 */
#define PREPROCESSOR_JOIN(x, y) PREPROCESSOR_JOIN_INNER(x, y)
#define PREPROCESSOR_JOIN_INNER(x, y) x##y

/**
 * Concatenates the first two arguments, and then again with the third argument
 */
#define PREPROCESSOR_JOIN_FIRST(x, ...) PREPROCESSOR_JOIN_FIRST_INNER(x, __VA_ARGS__)
#define PREPROCESSOR_JOIN_FIRST_INNER(x, ...) x##__VA_ARGS__

/**
 * Expands to the second argument or the third argument depending on whether the
 * first argument expands to 1 or 0
 */
#define PREPROCESSOR_IF(cond, x, y) PREPROCESSOR_JOIN(PREPROCESSOR_IF_INNER_, cond)(x, y)
#define PREPROCESSOR_IF_INNER_1(x, y) x
#define PREPROCESSOR_IF_INNER_0(x, y) y

/**
 * Expands to the parameter list of the macro - used for passing a comma-separated
 * list as a single macro argument
 */
#define PREPROCESSOR_COMMA_SEPARATED(first, second, ...) first, second, ##__VA_ARGS__

/**
 * Expands to nothing - used as a placeholder
 */
#define PREPROCESSOR_NOTHING

/**
 * Removes a single layer of parentheses from a macro argument if they are present
 */
#define PREPROCESSOR_REMOVE_OPTIONAL_PARENS(...) PREPROCESSOR_JOIN_FIRST(PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL, PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL __VA_ARGS__)
#define PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL(...) PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL __VA_ARGS__
#define PREPROCESSOR_REMOVE_OPTIONAL_PARENSPREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL
