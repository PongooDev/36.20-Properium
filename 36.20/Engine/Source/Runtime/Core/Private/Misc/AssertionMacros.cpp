#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Misc/AssertionMacros.h"

#include <cstdarg>
#include <cstdio>

void VARARGS FDebug::LogAssertFailedMessage(const ANSICHAR* Expr, const ANSICHAR* File, int32 Line, const TCHAR* Fmt, ...)
{
	TCHAR Message[4096];
	Message[0] = TEXT('\0');

	if (Fmt)
	{
		va_list Args;
		va_start(Args, Fmt);
		_vsnwprintf_s(Message, _TRUNCATE, Fmt, Args);
		va_end(Args);
	}

	wprintf(TEXT("Assertion failed: %hs [File:%hs] [Line: %d]\n"), Expr, File ? File : "<unknown>", Line);

	if (Message[0] != TEXT('\0'))
		wprintf(TEXT("%s\n"), Message);

	fflush(stdout);
}
