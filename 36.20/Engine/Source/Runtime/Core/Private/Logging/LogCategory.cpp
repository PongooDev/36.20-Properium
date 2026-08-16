// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogCategory.h"

FLogCategoryBase::FLogCategoryBase(const FLogCategoryName& InCategoryName, ELogVerbosity::Type InDefaultVerbosity, ELogVerbosity::Type InCompileTimeVerbosity)
	: Verbosity(uint8(InDefaultVerbosity))
	, DebugBreakOnLog(0)
	, DefaultVerbosity(uint8(InDefaultVerbosity))
	, CompileTimeVerbosity(uint8(InCompileTimeVerbosity))
	, CategoryName(InCategoryName)
{
	constexpr int32 FLogCategoryBase_Ctor = 0x2884E54;

	if (FLogCategoryBase_Ctor)
	{
		void (*Fn)(FLogCategoryBase*, const FLogCategoryName*, uint8, uint8) =
			decltype(Fn)(ImageBase + FLogCategoryBase_Ctor);

		Fn(this, &InCategoryName, uint8(InDefaultVerbosity), uint8(InCompileTimeVerbosity));
		return;
	}

	ResetFromDefault();
}

static FLogCategoryName MakeCategoryName(const TCHAR* InCategoryName)
{
	constexpr int32 FName_FromWide = 0x1847508;

	FLogCategoryName Out;

	if constexpr (FName_FromWide != 0)
	{
		void (*Fn)(FLogCategoryName*, const TCHAR*, int32) = decltype(Fn)(ImageBase + FName_FromWide);
		Fn(&Out, InCategoryName, 1);
	}

	return Out;
}

FLogCategoryBase::FLogCategoryBase(const TCHAR* InCategoryName, ELogVerbosity::Type InDefaultVerbosity, ELogVerbosity::Type InCompileTimeVerbosity)
	: FLogCategoryBase(MakeCategoryName(InCategoryName), InDefaultVerbosity, InCompileTimeVerbosity)
{
}

FLogCategoryBase::~FLogCategoryBase()
{
	if (CompileTimeVerbosity > ELogVerbosity::NoLogging)
	{
		constexpr int32 GLogSuppressionInterface = 0x14F8CD30;
		constexpr int32 FLogSuppressionInterface_DisassociateSuppress = 0x65AFE1C;

		if (GLogSuppressionInterface && FLogSuppressionInterface_DisassociateSuppress)
		{
			void (*DisassociateSuppress)(void*, FLogCategoryBase*) = decltype(DisassociateSuppress)(ImageBase + FLogSuppressionInterface_DisassociateSuppress);

			if (void* Singleton = *reinterpret_cast<void**>(ImageBase + GLogSuppressionInterface))
			{
				DisassociateSuppress(Singleton, this);
			}
		}
	}
}

void FLogCategoryBase::SetVerbosity(ELogVerbosity::Type NewVerbosity)
{
	constexpr int32 FLogCategoryBase_SetVerbosity = 0x35755C8;

	if (FLogCategoryBase_SetVerbosity)
	{
		void (*Fn)(FLogCategoryBase*, uint8) = decltype(Fn)(ImageBase + FLogCategoryBase_SetVerbosity);

		Fn(this, uint8(NewVerbosity));
		return;
	}

	const uint8 Requested = uint8(NewVerbosity & ELogVerbosity::VerbosityMask);
	Verbosity = Requested < CompileTimeVerbosity ? Requested : CompileTimeVerbosity;
	DebugBreakOnLog = !!(NewVerbosity & ELogVerbosity::BreakOnLog);
}

void FLogCategoryBase::ResetFromDefault()
{
	SetVerbosity(ELogVerbosity::Type(DefaultVerbosity));
}

void FLogCategoryBase::PostTrigger(ELogVerbosity::Type VerbosityLevel)
{
	if (DebugBreakOnLog || (VerbosityLevel & ELogVerbosity::BreakOnLog))
	{
		constexpr int32 FOutputDeviceRedirector_Get = 0x1DCBE08;
		constexpr int32 FOutputDeviceRedirector_FlushThreadedLogs = 0x1DCBF30;

		if (FOutputDeviceRedirector_Get && FOutputDeviceRedirector_FlushThreadedLogs)
		{
			void* (*Get)() = decltype(Get)(ImageBase + FOutputDeviceRedirector_Get);
			void (*FlushThreadedLogs)(void*, uint8) = decltype(FlushThreadedLogs)(ImageBase + FOutputDeviceRedirector_FlushThreadedLogs);

			if (void* Redirector = Get())
			{
				FlushThreadedLogs(Redirector, 0);
			}
		}

		DebugBreakOnLog = false;

		if (IsDebuggerPresent())
		{
			PLATFORM_BREAK();
		}
	}
}
