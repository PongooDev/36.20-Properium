#include "pch.h"

void AFortGameModeAthena::FinishWorldInitializationHook(AFortGameModeAthena* This, AFortWorldManager* WorldManager) {
	AFortGameModeZone::FinishWorldInitializationHook(This, WorldManager);
	FinishWorldInitializationOG(This, WorldManager);
}

bool AFortGameModeAthena::ReadyToStartMatchHook(AFortGameModeAthena* This) {
	if (This->bWorldIsReady
		&& This->MatchState == L"WaitingToStart"
		&& This->NumPlayers >= This->WarmupRequiredPlayerCount) {

		return This->CountReadyPlayers();
	}

	return false;
}

UClass** AFortGameModeAthena::GetGameSessionClassHook(AFortGameModeAthena* This, UClass** result) {
	if (!Configuration::bUseGameSessions) return GetGameSessionClassOG(This, result);

	*result = AFortGameSessionDedicatedAthena::StaticClass();
	This->GameSessionClass = *result;

	return result;
}

APawn* AFortGameModeAthena::SpawnDefaultPawnForHook(AFortGameModeAthena* This, AController* NewPlayer, AActor* StartSpot) {
	if (!StartSpot) {
		UE_LOG_REF(LogFort, Warning, TEXT("SpawnDefaultPawnFor: StartSpot is null"));
		return nullptr;
	}

	if (!NewPlayer) {
		UE_LOG_REF(LogFort, Warning, TEXT("SpawnDefaultPawnFor: NewPlayer is null"));
		return nullptr;
	}

	UClass* PawnClass = This->GetDefaultPawnClassForController(NewPlayer);
	if (!PawnClass) {
		UE_LOG_REF(LogFort, Warning, TEXT("SpawnDefaultPawnFor: no default pawn class for %hs"), NewPlayer->GetName().c_str());
		return nullptr;
	}

	double DefaultHalfHeight = 0.0;
	if (APawn* PawnDefaultObject = static_cast<APawn*>(PawnClass->DefaultObject)) {
		constexpr int32 GetDefaultHalfHeightVfIdx = 241;

		auto GetDefaultHalfHeight = reinterpret_cast<float(*)(APawn*)>(PawnDefaultObject->VTable[GetDefaultHalfHeightVfIdx]);
		DefaultHalfHeight = GetDefaultHalfHeight(PawnDefaultObject);
	}

	AActor* SpawnSpot = StartSpot;
	APawn* ResultPawn = nullptr;

	for (int32 Attempt = 0; Attempt <= 4 && !ResultPawn; ++Attempt) {
		if (!SpawnSpot)
			break;

		const FRotator SpotRotation = SpawnSpot->K2_GetActorRotation();

		FVector SpawnLocation = SpawnSpot->K2_GetActorLocation();
		SpawnLocation.Z += DefaultHalfHeight + 24.0;
			
		bool bSpawnLocationBlocked = false;

		if (AGameStateBase* GameState = This->GameState) {
			for (APlayerState* PlayerState : GameState->PlayerArray) {
				if (!PlayerState)
					continue;

				APawn* Pawn = PlayerState->PawnPrivate;
				if (!Pawn || Pawn->Controller == NewPlayer)
					continue;

				const FVector PawnLocation = Pawn->K2_GetActorLocation();

				const double DeltaX = PawnLocation.X - SpawnLocation.X;
				const double DeltaY = PawnLocation.Y - SpawnLocation.Y;

				if (((DeltaX * DeltaX) + (DeltaY * DeltaY)) < 36.0) {
					bSpawnLocationBlocked = true;
					break;
				}
			}
		}

		if (!bSpawnLocationBlocked) {
			const double HalfYawRadians = SpotRotation.Yaw * (3.1415926535897932 / 180.0) * 0.5;

			FTransform SpawnTransform{};
			SpawnTransform.Rotation.X = 0.0;
			SpawnTransform.Rotation.Y = 0.0;
			SpawnTransform.Rotation.Z = sin(HalfYawRadians);
			SpawnTransform.Rotation.W = cos(HalfYawRadians);
			SpawnTransform.Translation = SpawnLocation;
			SpawnTransform.Scale3D.X = 1.0;
			SpawnTransform.Scale3D.Y = 1.0;
			SpawnTransform.Scale3D.Z = 1.0;

			FActorSpawnParameters SpawnInfo;
			SpawnInfo.ObjectFlags |= 0x40;
			SpawnInfo.bDeferConstruction = 1;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ResultPawn = static_cast<APawn*>(GWorld->SpawnActor(PawnClass, &SpawnTransform, SpawnInfo));

			if (ResultPawn) {
				UGameplayStatics::FinishSpawningActor(ResultPawn, SpawnTransform, ESpawnActorScaleMethod::MultiplyWithRoot);
				break;
			}
		}

		SpawnSpot = This->ChoosePlayerStart(NewPlayer);
	}

	if (!ResultPawn) {
		UE_LOG_REF(LogFort, Warning, TEXT("SpawnDefaultPawnFor: failed to spawn %hs for %hs"),
			PawnClass->GetName().c_str(), NewPlayer->GetName().c_str());
	}

	return ResultPawn;
}

void AFortGameModeAthena::InitGameStateHook(AFortGameModeAthena* This) {
	InitGameStateOG(This);

	AFortGameStateAthena* GameState = This->GameState->Cast<AFortGameStateAthena>();
	if (!GameState) {
		return;
	}

	UFortPlaylistAthena* Playlist = UObject::StaticLoadObject(
		UFortPlaylistAthena::StaticClass(),
		nullptr,
		Configuration::Playlist.c_str()
	)->Cast<UFortPlaylistAthena>();

	if (!Playlist) {
		return;
	}

	This->GameSession->MaxPlayers = Playlist->MaxPlayers;
	This->GameSession->MaxPartySize = Playlist->MaxTeamSize;

	This->MaxPlayerCount = Playlist->MaxPlayers;

	This->SetCurrentPlaylistName(Playlist->PlaylistName);
	This->SetCurrentPlaylistId(Playlist->PlaylistId);

	GameState->bStormReachedFinalPosition = false;

	GameState->SetCurrentPlaylistId(Playlist->PlaylistId);

	GameState->CurrentPlaylistInfo.SetBasePlaylist(Playlist);
	GameState->CurrentPlaylistInfo.MarkArrayDirty();

	GameState->TeamCount = Playlist->MaxTeamCount;

	GameState->OnRep_CurrentPlaylistInfo();
}

void AFortGameModeAthena::Init() {
	Memory::HookDetour(ImageBase + 0xB22FE64, FinishWorldInitializationHook, &FinishWorldInitializationOG);
	Memory::HookDetour(ImageBase + 0xB23A644, ReadyToStartMatchHook);
	Memory::HookDetour(ImageBase + 0xB231A58, GetGameSessionClassHook, &GetGameSessionClassOG);
	Memory::HookDetour(ImageBase + 0xB23E1B0, SpawnDefaultPawnForHook);
	Memory::HookDetour(ImageBase + 0xB2358F4, InitGameStateHook, &InitGameStateOG);
}