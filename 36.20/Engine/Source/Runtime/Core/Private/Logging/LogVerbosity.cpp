#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogVerbosity.h"

const TCHAR* ToString(ELogVerbosity::Type Verbosity)
{
	const TCHAR* (*Fn)(ELogVerbosity::Type) = decltype(Fn)(ImageBase + 0x1AEFEC0);
	return Fn(Verbosity);
}

ELogVerbosity::Type ParseLogVerbosityFromString(const FString& VerbosityString)
{
	if (VerbosityString == TEXT("NoLogging"))
	{
		return ELogVerbosity::NoLogging;
	}
	else if (VerbosityString == TEXT("Fatal"))
	{
		return ELogVerbosity::Fatal;
	}
	else if (VerbosityString == TEXT("Error"))
	{
		return ELogVerbosity::Error;
	}
	else if (VerbosityString == TEXT("Warning"))
	{
		return ELogVerbosity::Warning;
	}
	else if (VerbosityString == TEXT("Display"))
	{
		return ELogVerbosity::Display;
	}
	else if (VerbosityString == TEXT("Log"))
	{
		return ELogVerbosity::Log;
	}
	else if (VerbosityString == TEXT("Verbose"))
	{
		return ELogVerbosity::Verbose;
	}
	else if (VerbosityString == TEXT("VeryVerbose"))
	{
		return ELogVerbosity::VeryVerbose;
	}
	else
	{
		// An unknown value is treated as log
		return ELogVerbosity::Log;
	}
}