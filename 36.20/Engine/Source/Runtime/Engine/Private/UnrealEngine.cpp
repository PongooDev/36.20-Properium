#include "pch.h"

bool UEngine::LoadMapHook(UEngine* This, FWorldContext& WorldContext, FURL& URL, class UPendingNetGame* Pending, FString& Error) {
	bool result = LoadMapOG(This, WorldContext, URL, Pending, Error);

	// Get last URL so we can still use the URL
	URL = WorldContext.LastURL;

	// Listen for clients.
	if (Pending == NULL && (!GIsClient || URL.HasOption(TEXT("Listen"))))
	{
		if (!WorldContext.World()->Listen(URL))
		{
			UE_LOG_REF(LogNet, Error, TEXT("LoadMap: failed to Listen(%s)"), URL.ToString());
		}
	}

	return result;
}

void UEngine::Init() {
	Memory::HookDetour(ImageBase + 0x2D047C0, LoadMapHook, &LoadMapOG);
	Memory::NopFunctionCall(ImageBase + 0x2D05C4B); // "LoadMap: failed to Listen(%s)" Log in LoadMap
}