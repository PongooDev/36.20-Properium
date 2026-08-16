// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogMacros.h"
#include "CrashReporter/Public/CrashReporter.h"

DWORD WINAPI Main(LPVOID) {
    FCrashReporter::Register();

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

    GIsClient = 0;
    GIsServer = 1;

    UKismetSystemLibrary::ExecuteConsoleCommand(GWorld, L"log LogConfig off", 0);
    UKismetSystemLibrary::ExecuteConsoleCommand(GWorld, L"log LogFortUIDirector off", 0);
    
    UWorld* World = UWorld::GetWorld();
    UFortGameInstance* FortGameInstance = World->OwningGameInstance->Cast<UFortGameInstance>();

    FString TravelURL = L"Asteria_Terrain?listen?RequiredPlayers=1";
    
    FortGameInstance ? FortGameInstance->LocalPlayers.Remove(0) : 0;

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
