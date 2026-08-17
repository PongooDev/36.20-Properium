#include "pch.h"

EFortPlayerValidationType AFortGameSession::ValidatePlayerHook(AFortGameSession* This, const FUniqueNetIdRepl* UniqueId, bool bIsLocalPlayer, FText* ReturnReason) {
	if (Configuration::bUseGameSessions) {
		return ValidatePlayerOG(This, UniqueId, bIsLocalPlayer, ReturnReason);
	}
	else {
		return EFortPlayerValidationType::ValidatedPlayer;
	}
}

void AFortGameSession::Init() {
	Memory::HookDetour(ImageBase + 0xAA7F5E4, ValidatePlayerHook, ValidatePlayerOG);
}