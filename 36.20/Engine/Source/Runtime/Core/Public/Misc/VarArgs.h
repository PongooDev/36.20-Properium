// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

#include <stdarg.h>

#ifndef VARARGS
	#define VARARGS __cdecl
#endif

#define GET_TYPED_VARARGS_RESULT(CharType, Buffer, BufferCount, StartIndex, Format, Result) \
	{ \
		va_list ArgPtr; \
		va_start(ArgPtr, Format); \
		Result = _vsnwprintf_s(Buffer, BufferCount, _TRUNCATE, Format, ArgPtr); \
		va_end(ArgPtr); \
	}

#define GET_VARARGS_RESULT(Buffer, BufferCount, StartIndex, Format, Result) \
	GET_TYPED_VARARGS_RESULT(TCHAR, Buffer, BufferCount, StartIndex, Format, Result)
