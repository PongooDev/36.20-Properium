#include "pch.h"

void UUserWidget::Init() {
	Memory::HookDetour(ImageBase + 0x26B5944, False); // UUserWidget* UUserWidget::CreateInstanceInternal(UObject* Outer, TSubclassOf<UUserWidget> UserWidgetClass, FName InstanceName, UWorld* World, ULocalPlayer* LocalPlayer);
}