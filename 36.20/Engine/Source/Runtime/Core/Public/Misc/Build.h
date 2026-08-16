// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#ifndef UE_BUILD_DEBUG
	#define UE_BUILD_DEBUG 0
#endif

#ifndef UE_BUILD_DEVELOPMENT
	#define UE_BUILD_DEVELOPMENT 1
#endif

#ifndef UE_BUILD_TEST
	#define UE_BUILD_TEST 0
#endif

#ifndef UE_BUILD_SHIPPING
	#define UE_BUILD_SHIPPING 0
#endif

#ifndef DO_CHECK
	#define DO_CHECK 1
#endif

#ifndef DO_GUARD_SLOW
	#define DO_GUARD_SLOW 0
#endif

#ifndef NO_LOGGING
	#define NO_LOGGING 0
#endif

#ifndef UE_VALIDATE_FORMAT_STRINGS
	#define UE_VALIDATE_FORMAT_STRINGS 0
#endif

#ifndef IS_MONOLITHIC
	#define IS_MONOLITHIC 1
#endif

#ifndef USING_CODE_ANALYSIS
	#define USING_CODE_ANALYSIS 0
#endif
