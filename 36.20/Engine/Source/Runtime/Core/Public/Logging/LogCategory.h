// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogVerbosity.h"

using FLogCategoryName = FName;

/** Base class for all log categories. **/
struct FLogCategoryBase
{
	/**
	 * Constructor, registers with the log suppression system and sets up the default values.
	 * @param InCategoryName, name of the category
	 * @param InDefaultVerbosity, default verbosity for the category, may be ignored and overridden by many other mechanisms
	 * @param InCompileTimeVerbosity, mostly used to keep the working verbosity in bounds, macros elsewhere actually do the work of stripping compiled out things.
	**/
	CORE_API FLogCategoryBase(const FLogCategoryName& InCategoryName, ELogVerbosity::Type InDefaultVerbosity, ELogVerbosity::Type InCompileTimeVerbosity);

	CORE_API FLogCategoryBase(const TCHAR* InCategoryName, ELogVerbosity::Type InDefaultVerbosity, ELogVerbosity::Type InCompileTimeVerbosity);

	/** Destructor, unregisters from the log suppression system **/
	CORE_API ~FLogCategoryBase();

	/** Should not generally be used directly. Tests the runtime verbosity. **/
	FORCEINLINE bool IsSuppressed(ELogVerbosity::Type VerbosityLevel) const
	{
		return !((VerbosityLevel & ELogVerbosity::VerbosityMask) <= Verbosity);
	}

	/** Called just after a logging statement being allowed to print. Checks a few things and maybe breaks into the debugger. **/
	CORE_API void PostTrigger(ELogVerbosity::Type VerbosityLevel);

	FORCEINLINE FLogCategoryName GetCategoryName() const { return CategoryName; }

	/** Gets the working verbosity **/
	FORCEINLINE ELogVerbosity::Type GetVerbosity() const { return (ELogVerbosity::Type)Verbosity; }

	/** Gets the compile time verbosity **/
	FORCEINLINE ELogVerbosity::Type GetCompileTimeVerbosity() const { return (ELogVerbosity::Type)CompileTimeVerbosity; }

	/** Sets up the working verbosity and clamps to the compile time verbosity. **/
	CORE_API void SetVerbosity(ELogVerbosity::Type Verbosity);

	/** Resets the working verbosity to the default this category was declared with. **/
	CORE_API void ResetFromDefault();

protected:
	/** Holds the current suppression state **/
	uint8 Verbosity;

	/** Holds the break flag **/
	uint8 DebugBreakOnLog;

	/** Verbosity this category was declared with **/
	const uint8 DefaultVerbosity;

	/** Verbosity above which this category is compiled out entirely **/
	const uint8 CompileTimeVerbosity;

private:
	/** Name of this category **/
	const FLogCategoryName CategoryName;
};

/**
 * Template for log categories that transfers the compile-time constant default
 * and compile time verbosity to the FLogCategoryBase constructor.
 **/
template <ELogVerbosity::Type InDefaultVerbosity, ELogVerbosity::Type InCompileTimeVerbosity>
struct FLogCategory : public FLogCategoryBase
{
	static_assert((InDefaultVerbosity & ELogVerbosity::VerbosityMask) < ELogVerbosity::NumVerbosity, "Bad default verbosity.");
	static_assert(!(InCompileTimeVerbosity & ELogVerbosity::BreakOnLog), "Bad compile time verbosity.");

	static constexpr ELogVerbosity::Type CompileTimeVerbosity = InCompileTimeVerbosity;
	static constexpr ELogVerbosity::Type DefaultVerbosity = InDefaultVerbosity;

	static constexpr ELogVerbosity::Type GetCompileTimeVerbosity() { return InCompileTimeVerbosity; }

	FORCEINLINE FLogCategory(const FLogCategoryName& InCategoryName)
		: FLogCategoryBase(InCategoryName, InDefaultVerbosity, InCompileTimeVerbosity)
	{
	}

	FORCEINLINE FLogCategory(const TCHAR* InCategoryName)
		: FLogCategoryBase(InCategoryName, InDefaultVerbosity, InCompileTimeVerbosity)
	{
	}
};
