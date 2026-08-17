// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogMacros.h"
#include "CrashReporter/Public/CrashReporter.h"

DWORD WINAPI Main(LPVOID) {
    FCrashReporter::Register();

    if (Configuration::bClient) {
        UObject* ConsoleObj = UGameplayStatics::SpawnObject(UEngine::GetEngine()->ConsoleClass, UEngine::GetEngine()->GameViewport);
        if (ConsoleObj) {
            UEngine::GetEngine()->GameViewport->ViewportConsole = (UConsole*)ConsoleObj;
            Log("Initialized Console!");
        }
        else {
            Log("Failed to Spawn Console!");
        }

        std::thread([]() {
            while (true)
            {
                if (GetAsyncKeyState(VK_F2))
                {
                    auto GameInstance = UWorld::GetWorld()->OwningGameInstance;
                    auto LocalPlayers = GameInstance->LocalPlayers;
                    auto PlayerController = LocalPlayers[0]->PlayerController;

                    PlayerController->CheatManager = (UFortCheatManager*)UGameplayStatics::SpawnObject(UFortCheatManager::StaticClass(), PlayerController);
                }
            }
            }).detach();

        Memory::Patch(ImageBase + 0x5841D50, 0xC3); // void AFortPlayerControllerFrontEnd::ShowAppEnvironmentSecurityMessage(AFortPlayerControllerFrontEnd* this, unsigned __int8 Category, FString* Details, bool bCloseClient);

        return 0;
    }

    InitConsole();
    Log(L"Welcome to 36.20-Properium!");
    Log(std::format("ImageBase: 0x{:X}", ImageBase));

    while (true)
    {
        if (GWorld)
        {
            if (GWorld->GetName() == "Frontend")
            {
                if (GWorld->AuthorityGameMode)
                {
                    if (auto GM = GWorld->AuthorityGameMode->Cast<AGameMode>())
                    {
                        if (GM->GetMatchState() == FName(L"InProgress"))
                        {
                            break;
                        }
                    }
                }
            }
        }

        Sleep(1000);
    }

    //Sleep(10000);

    UUserWidget::Init();
    AFortPlayerControllerFrontEnd::Init();

    if (GIsClient) {
        GIsClient = 0;
    }
    if (!GIsServer) {
        GIsServer = 1;
    }

    UKismetSystemLibrary::ExecuteConsoleCommand(GWorld, L"log LogConfig off", 0);
    UKismetSystemLibrary::ExecuteConsoleCommand(GWorld, L"log LogFortUIDirector off", 0);
    
    UWorld* World = UWorld::GetWorld();
    if (World) {
        UFortGameInstance* FortGameInstance = World->OwningGameInstance->Cast<UFortGameInstance>();

        FString TravelURL = L"Hermes_Terrain?listen?RequiredPlayers=1";

        if (FortGameInstance && FortGameInstance->LocalPlayers.Num()) {
            FortGameInstance->LocalPlayers.Remove(0);
        }
        World->ServerTravel(TravelURL);
    }

    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, Main, 0, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
