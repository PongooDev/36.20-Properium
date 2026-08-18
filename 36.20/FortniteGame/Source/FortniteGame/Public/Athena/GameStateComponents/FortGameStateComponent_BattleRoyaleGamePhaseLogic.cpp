#include "pch.h"

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::TickComponentHook(UFortGameStateComponent_BattleRoyaleGamePhaseLogic* This, float DeltaTime, int32 TickType, void* ThisTickFunction) {
	if (This->GamePhase < EAthenaGamePhase::Warmup) {
		AFortGameStateAthena* GameState = static_cast<AFortGameStateAthena*>(This->GetOwner());

		if (GameState) {
			const EAthenaGamePhase OldGamePhase = This->GamePhase;
			const float ServerTime = static_cast<float>(GameState->GetServerWorldTimeSeconds());

			This->WarmupCountdownStartTime = ServerTime;
			This->WarmupCountdownEndTime = ServerTime + This->WarmupCountdownDuration;
			This->GamePhase = EAthenaGamePhase::Warmup;

			This->OnRep_GamePhase(OldGamePhase);
			This->OnRep_WarmupCountdownEndTime();

			UE_LOG_REF(LogFort, Warning, TEXT("GamePhase: %d -> Warmup (ServerTime=%f Duration=%f End=%f)"),
				static_cast<int32>(OldGamePhase), ServerTime, This->WarmupCountdownDuration, This->WarmupCountdownEndTime);
		}
	}

	return TickComponentOG(This, DeltaTime, TickType, ThisTickFunction);
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Init() {
	Memory::HookDetour(ImageBase + 0x1C771F0, True); // bool UFortGameStateComponent_BattleRoyaleGamePhaseLogic::CanUpdateGamePhaseStep();
	Memory::HookDetour(ImageBase + 0x1C76AF0, TickComponentHook, &TickComponentOG);
}
