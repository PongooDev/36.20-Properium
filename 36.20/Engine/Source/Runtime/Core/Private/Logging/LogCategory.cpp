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
	constexpr int32 FLogCategoryBase_Ctor = 0x1ADDE84;

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
	constexpr int32 FName_FromWide = 0x10F702C;

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
		constexpr int32 FLogSuppressionInterface_TryGet = 0x0;
		constexpr int32 FLogSuppressionInterface_DisassociateSuppress = 0x0;

		if (FLogSuppressionInterface_TryGet && FLogSuppressionInterface_DisassociateSuppress)
		{
			void* (*TryGet)() = decltype(TryGet)(ImageBase + FLogSuppressionInterface_TryGet);
			void (*DisassociateSuppress)(void*, FLogCategoryBase*) = decltype(DisassociateSuppress)(ImageBase + FLogSuppressionInterface_DisassociateSuppress);

			if (void* Singleton = TryGet())
			{
				DisassociateSuppress(Singleton, this);
			}
		}
	}
}

void FLogCategoryBase::SetVerbosity(ELogVerbosity::Type NewVerbosity)
{
	constexpr int32 FLogCategoryBase_SetVerbosity = 0x1021D54;

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
		constexpr int32 GLog = 0x0;
		constexpr int32 FOutputDeviceRedirector_FlushThreadedLogs = 0x0;

		if (GLog && FOutputDeviceRedirector_FlushThreadedLogs)
		{
			void* Redirector = *reinterpret_cast<void**>(ImageBase + GLog);
			void (*FlushThreadedLogs)(void*, bool) = decltype(FlushThreadedLogs)(ImageBase + FOutputDeviceRedirector_FlushThreadedLogs);

			if (Redirector)
			{
				FlushThreadedLogs(Redirector, false);
			}
		}

		DebugBreakOnLog = false;

		if (IsDebuggerPresent())
		{
			PLATFORM_BREAK();
		}
	}
}
