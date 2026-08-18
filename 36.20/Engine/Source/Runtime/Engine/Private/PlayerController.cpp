#include "pch.h"

void APlayerController::Init() {
	Memory::CreateVTableOriginal(APlayerController::GetDefaultObj(), 0x135, (LPVOID*)&ServerAcknowledgePossessionOG);
}