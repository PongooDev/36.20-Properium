// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Misc/Build.h"
#include "Engine/Source/Runtime/Core/Public/Misc/VarArgs.h"

#ifndef CA_ASSUME
	#define CA_ASSUME(expr) __assume(expr)
#endif

#ifndef CA_CONSTANT_IF
	#define CA_CONSTANT_IF(Condition) __pragma(warning(suppress: 4127)) if (Condition)
#endif

#ifndef LowLevelFatalError
	#define LowLevelFatalError(Format, ...) \
		do \
		{ \
			wprintf(TEXT("Fatal error: ")); \
			wprintf(Format, ##__VA_ARGS__); \
			wprintf(TEXT("\n")); \
			fflush(stdout); \
			if (IsDebuggerPresent()) \
			{ \
				PLATFORM_BREAK(); \
			} \
			abort(); \
		} while (0)
#endif

struct FDebug
{
	static void VARARGS LogAssertFailedMessage(const ANSICHAR* Expr, const ANSICHAR* File, int32 Line, const TCHAR* Fmt = nullptr, ...);
};

#if DO_CHECK

	#define check(expr) \
		do \
		{ \
			if (!(expr)) \
			{ \
				FDebug::LogAssertFailedMessage(#expr, __FILE__, __LINE__); \
				PLATFORM_BREAK(); \
				CA_ASSUME(false); \
			} \
		} while (0)

	#define checkf(expr, format, ...) \
		do \
		{ \
			if (!(expr)) \
			{ \
				FDebug::LogAssertFailedMessage(#expr, __FILE__, __LINE__, format, ##__VA_ARGS__); \
				PLATFORM_BREAK(); \
				CA_ASSUME(false); \
			} \
		} while (0)

	#define verify(expr)                 check(expr)
	#define verifyf(expr, format, ...)   checkf(expr, format, ##__VA_ARGS__)

#else

	#define check(expr)                  do { CA_ASSUME(expr); } while (0)
	#define checkf(expr, format, ...)    do { CA_ASSUME(expr); } while (0)
	#define verify(expr)                 do { if (!(expr)) {} } while (0)
	#define verifyf(expr, format, ...)   do { if (!(expr)) {} } while (0)

#endif

#define checkSlow(expr)                  check(expr)
#define checkfSlow(expr, format, ...)    checkf(expr, format, ##__VA_ARGS__)
#define checkNoEntry()                   checkf(false, TEXT("Enclosing block should never be called"))
