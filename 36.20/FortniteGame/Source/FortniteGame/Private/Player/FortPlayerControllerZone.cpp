#include "pch.h"

void AFortPlayerControllerZone::ServerAcknowledgePossessionHook(AFortPlayerControllerZone* This, APawn* P)
{
	APlayerController::ServerAcknowledgePossessionOG(This, P);
}

void AFortPlayerControllerZone::Init() {
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerControllerZone>(0x135, ServerAcknowledgePossessionHook);
}