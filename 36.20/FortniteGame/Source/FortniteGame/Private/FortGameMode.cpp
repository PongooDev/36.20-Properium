#include "pch.h"

void AFortGameMode::FinishWorldInitializationHook(AFortGameMode* This, AFortWorldManager* WorldManager) {
	FinishWorldInitializationOG(This, WorldManager);
}

UClass** AFortGameMode::GetGameSessionClassHook(AFortGameMode* This, UClass** result) {
	if (!Configuration::bUseGameSessions) return GetGameSessionClassOG(This, result);

	*result = AFortGameSessionDedicated::StaticClass();
	This->GameSessionClass = *result;

	return result;
}

void AFortGameMode::Init() {
	Memory::HookDetour(ImageBase + 0xA5CD60C, FinishWorldInitializationHook, &FinishWorldInitializationOG);
	Memory::HookDetour(ImageBase + 0x40476A8, GetGameSessionClassHook, &GetGameSessionClassOG);
}