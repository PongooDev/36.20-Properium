// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

typedef char     ANSICHAR;
typedef wchar_t  WIDECHAR;
typedef uint8_t  UTF8CHAR;

#ifndef CORE_API
	#define CORE_API
#endif

#ifndef FORCEINLINE
	#define FORCEINLINE __forceinline
#endif

#ifndef FORCENOINLINE
	#define FORCENOINLINE __declspec(noinline)
#endif

#ifndef PLATFORM_BREAK
	#define PLATFORM_BREAK() __debugbreak()
#endif
