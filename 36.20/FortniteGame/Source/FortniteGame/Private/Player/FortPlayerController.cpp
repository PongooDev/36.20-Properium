#include "pch.h"

void AFortPlayerController::RebuildInventoriesIfNeededHook(AFortPlayerController* This) {
	UClass* WorldInventoryClass = This->WorldInventoryClass;
	if (!WorldInventoryClass) {
		WorldInventoryClass = AFortInventory::StaticClass();
	}

	FTransform SpawnTransform{};
	SpawnTransform.Rotation.W = 1.0;
	SpawnTransform.Scale3D.X = 1.0;
	SpawnTransform.Scale3D.Y = 1.0;
	SpawnTransform.Scale3D.Z = 1.0;

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = This;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFortInventory* WorldInventory = (AFortInventory*)GWorld->SpawnActor(WorldInventoryClass, &SpawnTransform, SpawnInfo);
	if (!WorldInventory) {
		return RebuildInventoriesIfNeededOG(This);
	}

	void* InterfaceAddress = WorldInventory->GetInterfaceAddress(IFortInventoryInterface::StaticClass());
	if (InterfaceAddress) {
		This->SetWorldInventory(InterfaceAddress);
	}

	return RebuildInventoriesIfNeededOG(This);
}

void AFortPlayerController::Init() {
	Memory::HookDetour(ImageBase + 0xB40994C, RebuildInventoriesIfNeededHook, &RebuildInventoriesIfNeededOG);
}
