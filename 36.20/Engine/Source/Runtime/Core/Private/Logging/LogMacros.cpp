// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogMacros.h"

namespace
{
	constexpr int32 GFormatBufferSize = 8192;

	void FallbackSerialize(const FLogCategoryName& Category, ELogVerbosity::Type Verbosity, const TCHAR* Message)
	{
		SYSTEMTIME Time{};
		GetLocalTime(&Time);

		const bool bPrintVerbosity = (Verbosity & ELogVerbosity::VerbosityMask) != ELogVerbosity::Log;

		const std::string CategoryName = const_cast<FLogCategoryName&>(Category).ToString();

		TCHAR Line[GFormatBufferSize];
		_snwprintf_s(Line, _TRUNCATE, TEXT("[%04d.%02d.%02d-%02d.%02d.%02d:%03d]%hs: %s%s%s"),
			Time.wYear, Time.wMonth, Time.wDay,
			Time.wHour, Time.wMinute, Time.wSecond, Time.wMilliseconds,
			CategoryName.c_str(),
			bPrintVerbosity ? ToString(Verbosity) : TEXT(""),
			bPrintVerbosity ? TEXT(": ") : TEXT(""),
			Message);

		wprintf(TEXT("%s\n"), Line);
		OutputDebugStringW(Line);
		OutputDebugStringW(TEXT("\n"));
	}
}

void FMsg::LogV(const ANSICHAR* File, int32 Line, const FLogCategoryName& Category, ELogVerbosity::Type Verbosity, const TCHAR* Fmt, va_list Args)
{
#if !NO_LOGGING
	constexpr int32 FMsg_LogV = 0x219895C;

	if (FMsg_LogV)
	{
		void (*Fn)(const ANSICHAR*, int32, const FLogCategoryName&, ELogVerbosity::Type, const TCHAR*, va_list) =
			decltype(Fn)(ImageBase + FMsg_LogV);

		Fn(File, Line, Category, Verbosity, Fmt, Args);
		return;
	}

	TCHAR Buffer[GFormatBufferSize];
	if (_vsnwprintf_s(Buffer, GFormatBufferSize, _TRUNCATE, Fmt, Args) < 0)
	{
		FallbackSerialize(Category, Verbosity, Fmt);
		return;
	}

	if ((Verbosity & ELogVerbosity::VerbosityMask) == ELogVerbosity::Fatal)
	{
		FallbackSerialize(Category, ELogVerbosity::Fatal, Buffer);
		fflush(stdout);

		constexpr int32 FDebug_AssertFailedV = 0x65C18CC;

		if (FDebug_AssertFailedV)
		{
			void (*AssertFailedV)(const ANSICHAR*, const ANSICHAR*, int32, void*, const TCHAR*, va_list) =
				decltype(AssertFailedV)(ImageBase + FDebug_AssertFailedV);

			AssertFailedV("", File, Line, nullptr, Fmt, Args);
		}

		if (IsDebuggerPresent())
		{
			PLATFORM_BREAK();
		}

		abort();
	}

	FallbackSerialize(Category, Verbosity, Buffer);
#endif
}

void VARARGS FMsg::LogfImpl(const ANSICHAR* File, int32 Line, const FLogCategoryName& Category, ELogVerbosity::Type Verbosity, const TCHAR* Fmt, ...)
{
#if !NO_LOGGING
	va_list Args;
	va_start(Args, Fmt);
	LogV(File, Line, Category, Verbosity, Fmt, Args);
	va_end(Args);
#endif
}

void VARARGS FMsg::Logf_InternalImpl(const ANSICHAR* File, int32 Line, const FLogCategoryName& Category, ELogVerbosity::Type Verbosity, const TCHAR* Fmt, ...)
{
#if !NO_LOGGING
	va_list Args;
	va_start(Args, Fmt);
	LogV(File, Line, Category, Verbosity, Fmt, Args);
	va_end(Args);
#endif
}

void VARARGS FMsg::SendNotificationStringfImpl(const TCHAR* Fmt, ...)
{
	TCHAR Buffer[GFormatBufferSize];

	va_list Args;
	va_start(Args, Fmt);
	const int Written = _vsnwprintf_s(Buffer, GFormatBufferSize, _TRUNCATE, Fmt, Args);
	va_end(Args);

	SendNotificationString(Written < 0 ? Fmt : Buffer);
}

void FMsg::SendNotificationString(const TCHAR* Message)
{
	constexpr int32 FPlatformMisc_LowLevelOutputDebugString = 0x0;

	if (FPlatformMisc_LowLevelOutputDebugString)
	{
		void (*LowLevelOutputDebugString)(const TCHAR*) =
			decltype(LowLevelOutputDebugString)(ImageBase + FPlatformMisc_LowLevelOutputDebugString);

		LowLevelOutputDebugString(Message);
		return;
	}

	OutputDebugStringW(Message);
}

namespace UE::Logging::Private
{

void BasicLog(const FLogCategoryBase& Category, const FStaticBasicLogRecord* Log, ...)
{
	va_list Args;
	va_start(Args, Log);
	FMsg::LogV(Log->File, Log->Line, Category.GetCategoryName(), Log->Verbosity, Log->Format, Args);
	va_end(Args);
}

void BasicFatalLog(const FLogCategoryBase& Category, const FStaticBasicLogRecord* Log, ...)
{
	va_list Args;
	va_start(Args, Log);
	FMsg::LogV(Log->File, Log->Line, Category.GetCategoryName(), ELogVerbosity::Fatal, Log->Format, Args);
	va_end(Args);
}

} // UE::Logging::Private
