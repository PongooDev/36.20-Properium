#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/HAL/ThreadHeartBeat.h"

void FThreadHeartBeat::Init() {
	Memory::HookDetour(ImageBase + 0x56E1790, False); // bool FThreadHeartBeat::IsEnabled();
}