#include "pch.h"

void AFortPlayerControllerFrontEnd::Init() {
	Memory::Patch(ImageBase + 0x5841D50, 0xC3); // void AFortPlayerControllerFrontEnd::ShowAppEnvironmentSecurityMessage(AFortPlayerControllerFrontEnd* this, unsigned __int8 Category, FString* Details, bool bCloseClient);
}