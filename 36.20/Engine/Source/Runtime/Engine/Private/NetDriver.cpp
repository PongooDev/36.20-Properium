#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Misc/AssertionMacros.h"

int32 UNetDriver::ServerReplicateActors(float DeltaSeconds)
{
	return 0;
}

void UNetDriver::SendClientMoveAdjustments()
{
	for (UNetConnection* Connection : ClientConnections)
	{
		if (Connection == nullptr || Connection->ViewTarget == nullptr)
		{
			continue;
		}

		if (APlayerController* PC = Connection->PlayerController)
		{
			PC->SendClientAdjustment();
		}

		for (UChildConnection* ChildConnection : Connection->Children)
		{
			if (ChildConnection == nullptr)
			{
				continue;
			}

			if (APlayerController* PC = ChildConnection->PlayerController)
			{
				PC->SendClientAdjustment();
			}
		}
	}
}

void UNetDriver::TickFlushHook(UNetDriver* This, float DeltaSeconds)
{
	if (This->IsServer() && This->ClientConnections.Num() > 0 && !This->bSkipServerReplicateActors)
	{
		// Update all clients.
		if (This->ReplicationSystem)
		{
			This->UpdateIrisReplicationViews();
			This->SendClientMoveAdjustments();
			This->NetUpdate(DeltaSeconds);
		}
		else
		{
			This->ServerReplicateActors(DeltaSeconds);
		}
	}

	TickFlushOG(This, DeltaSeconds);
}

void UNetDriver::Init() {
	Memory::HookDetour(ImageBase + 0x1F51D90, TickFlushHook, &TickFlushOG);
}