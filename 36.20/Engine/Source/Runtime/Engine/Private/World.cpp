#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Misc/Parse.h"

bool UWorld::Listen(FURL& InURL) {
	if (NetDriver)
	{
		GEngine->BroadcastNetworkFailure(this, NetDriver, ENetworkFailure::NetDriverAlreadyExists);
		return false;
	}

	// Create net driver.
	FName NetDriverDefinition = L"GameNetDriver";
	if (InURL.HasOption(TEXT("NetDriverDef=")))
	{
		NetDriverDefinition = InURL.GetOption(TEXT("NetDriverDef="), nullptr);
	}

	if (GEngine->CreateNamedNetDriver(this, L"GameNetDriver", NetDriverDefinition)) {
		NetDriver = GEngine->FindNamedNetDriver(this, L"GameNetDriver");
		NetDriver->SetWorld(this);
		FLevelCollection* const SourceCollection = FindCollectionByType(ELevelCollectionType::DynamicSourceLevels);
		if (SourceCollection)
		{
			SourceCollection->SetNetDriver(NetDriver);
		}
		FLevelCollection* const StaticCollection = FindCollectionByType(ELevelCollectionType::StaticLevels);
		if (StaticCollection)
		{
			StaticCollection->SetNetDriver(NetDriver);
		}
	}

	if (NetDriver == nullptr)
	{
		GEngine->BroadcastNetworkFailure(this, NULL, ENetworkFailure::NetDriverCreateFailure);
		return false;
	}

	AWorldSettings* WorldSettings = GetWorldSettings();
	const bool bReuseAddressAndPort = WorldSettings ? WorldSettings->bReuseAddressAndPort : false;

	FString Error;
	if (!NetDriver->InitListen(GetNetworkNotify(), InURL, bReuseAddressAndPort, Error)) {
		GEngine->BroadcastNetworkFailure(this, NetDriver, ENetworkFailure::NetDriverListenFailure, Error);
		UE_LOG_REF(LogWorld, Log, TEXT("Failed to listen: %s"), Error);
		GEngine->DestroyNamedNetDriver(this, NetDriver->NetDriverName);
		NetDriver = nullptr;
		FLevelCollection* SourceCollection = FindCollectionByType(ELevelCollectionType::DynamicSourceLevels);
		if (SourceCollection)
		{
			SourceCollection->SetNetDriver(nullptr);
		}
		FLevelCollection* StaticCollection = FindCollectionByType(ELevelCollectionType::StaticLevels);
		if (StaticCollection)
		{
			StaticCollection->SetNetDriver(nullptr);
		}
		return false;
	}
	static const bool bLanPlay = FParse::Param(GetCommandLineW(), TEXT("lanplay"));
	const bool bLanSpeed = bLanPlay || InURL.HasOption(TEXT("LAN"));
	if (!bLanSpeed && (NetDriver->MaxInternetClientRate < NetDriver->MaxClientRate) && (NetDriver->MaxInternetClientRate > 2500))
	{
		NetDriver->MaxClientRate = NetDriver->MaxInternetClientRate;
	}

	NextSwitchCountdown = NetDriver->ServerTravelPause;
	return true;
}

ENetMode UWorld::AttemptDeriveFromURLHook(UWorld* This) {
	return ENetMode::NM_DedicatedServer;
}

void UWorld::BeginPlayHook(UWorld* This) {
	This->ServerStreamingLevelsVisibility = AServerStreamingLevelsVisibility::SpawnServerActor(This);

	UE_LOG_REF(LogWorld, Log, TEXT("BeginPlay: ServerStreamingLevelsVisibility = 0x%llX"),
		reinterpret_cast<uintptr_t>(This->ServerStreamingLevelsVisibility));

	return BeginPlayOG(This);
}

void UWorld::Init() {
	PatchAllNetModes(ImageBase + 0x2D06570);
	Memory::HookDetour(ImageBase + 0x2D06570, AttemptDeriveFromURLHook, nullptr);
	Memory::HookDetour(ImageBase + 0x3158974, BeginPlayHook, &BeginPlayOG);
}