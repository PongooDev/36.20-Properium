// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <map>
#include <format>
#include <print>
#include <array>
#include <minmax.h>
#include <TlHelp32.h>
#include <dbghelp.h>
#include <sstream>
#include <winternl.h>
#include <cstdlib>
#include <cmath>
#include <intrin.h>
#include <windows.h>
#include <psapi.h>
#include <unordered_map>
#include <numeric>
#include <cwctype>
#include <cstdarg>
#include <unordered_set>

#include "minhook/MinHook.h"

#pragma comment(lib, "minhook/minhook.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ntdll.lib")

#include "SDK/SDK.hpp"

using namespace SDK;

namespace Configuration {
	inline bool bUseGameSessions = false;
}

inline uint64_t ImageBase = InSDKUtils::GetImageBase();

#define GWorld (*((UWorld**)(0x14E2C5F8 + InSDKUtils::GetImageBase())))
#define GEngine (*((UFortEngine**)(0x14EE2328  + InSDKUtils::GetImageBase())))
#define GIsClient (*(bool*)(0x14EA57DA + InSDKUtils::GetImageBase()))
#define GIsServer (*(bool*)(0x14EA5757 + InSDKUtils::GetImageBase()))

#include "Memcury/Memcury.h"

#include "Engine/Source/Runtime/Core/Public/Logging/LogMacros.h"

inline void InitConsole() {
	AllocConsole();
	FILE* fptr;
	freopen_s(&fptr, "CONOUT$", "w+", stdout);
}

inline FLogCategoryBase& LogProperium()
{
	static FLogCategory<ELogVerbosity::Log, ELogVerbosity::All> Category(TEXT("LogProperium"));
	return Category;
}

template <typename... Types>
inline void Log(ELogVerbosity::Type Verbosity, const TCHAR* Fmt, Types... Args)
{
	FLogCategoryBase& Category = LogProperium();

	if (!Category.IsSuppressed(Verbosity))
	{
		FMsg::Logf(__FILE__, __LINE__, Category.GetCategoryName(), Verbosity, Fmt, Args...);
	}
}

template <typename... Types>
inline void Log(const TCHAR* Fmt, Types... Args)
{
	Log(ELogVerbosity::Display, Fmt, Args...);
}

inline std::wstring ToWide(const char* Str)
{
	if (!Str || !*Str)
		return {};

	const int Length = MultiByteToWideChar(CP_UTF8, 0, Str, -1, nullptr, 0);
	if (Length <= 1)
		return {};

	std::wstring Out(static_cast<size_t>(Length) - 1, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, Str, -1, Out.data(), Length);
	return Out;
}

template <typename... Types>
inline void Log(ELogVerbosity::Type Verbosity, const char* Fmt, Types... Args)
{
	Log(Verbosity, ToWide(Fmt).c_str(), Args...);
}

template <typename... Types>
inline void Log(const char* Fmt, Types... Args)
{
	Log(ELogVerbosity::Display, ToWide(Fmt).c_str(), Args...);
}

inline void Log(ELogVerbosity::Type Verbosity, const std::string& Message)
{
	Log(Verbosity, TEXT("%hs"), Message.c_str());
}

inline void Log(const std::string& Message)
{
	Log(ELogVerbosity::Display, TEXT("%hs"), Message.c_str());
}

class Memory
{
public:
	template <typename _Is = uint8_t>
	static void Patch(uintptr_t ptr, _Is byte)
	{
		DWORD og;
		VirtualProtect(LPVOID(ptr), sizeof(_Is), PAGE_EXECUTE_READWRITE, &og);
		*(_Is*)ptr = byte;
		VirtualProtect(LPVOID(ptr), sizeof(_Is), og, &og);
	}

	static void Nop(uintptr_t ptr, size_t size)
	{
		DWORD og;
		VirtualProtect(LPVOID(ptr), size, PAGE_EXECUTE_READWRITE, &og);
		memset(LPVOID(ptr), 0x90, size);
		VirtualProtect(LPVOID(ptr), size, og, &og);

		FlushInstructionCache(GetCurrentProcess(), LPCVOID(ptr), size);
	}

	static void NopFunctionCall(uintptr_t callSite)
	{
		if (*reinterpret_cast<uint8_t*>(callSite) != 0xE8)
			return;

		Nop(callSite, 5);
	}

	static void SwapVTableEntry(void** VTable, int Idx, void* Detour, void** OG = 0)
	{
		DWORD oldProtection;

		if (OG)
		{
			*OG = VTable[Idx];
		}

		VirtualProtect(&VTable[Idx], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtection);

		VTable[Idx] = Detour;

		VirtualProtect(&VTable[Idx], sizeof(void*), oldProtection, NULL);
	}

	static void ChangeVFTCallClass(UClass* OriginalClass, uint32_t Idx, UClass* NewClass)
	{
		SwapVTableEntry(OriginalClass->DefaultObject->VTable, Idx, OriginalClass->DefaultObject->VTable[Idx]);
	}

	static void HookDetour(uintptr_t Ptr, void* Detour, void* Original = nullptr)
	{
		static bool check = false;
		if (!check)
		{
			MH_Initialize();
			check = !check;
		}
		MH_CreateHook(LPVOID(Ptr), Detour, (void**)Original);
		MH_EnableHook(LPVOID(Ptr));
	}

	static inline void ModifyInstructionLEA(uintptr_t instrAddr, uintptr_t targetAddr, int offset)
	{
		int64_t delta = static_cast<int64_t>(targetAddr) -
			static_cast<int64_t>(instrAddr + offset + 4);

		auto patchLocation = reinterpret_cast<int32_t*>(instrAddr + offset);

		DWORD oldProtect;
		VirtualProtect(patchLocation, sizeof(int32_t), PAGE_EXECUTE_READWRITE, &oldProtect);

		*patchLocation = static_cast<int32_t>(delta);

		DWORD temp;
		VirtualProtect(patchLocation, sizeof(int32_t), oldProtect, &temp);
	}

	static void HookUFunction(UFunction* Function, void* Detour, void** Original = nullptr)
	{
		if (Original) *Original = (void*)Function->ExecFunction;
		Function->ExecFunction = reinterpret_cast<UFunction::FNativeFuncPtr>(Detour);
	}

	template<typename T>
	static void SwapVTableEntryInAllSubClasses(uint32_t Idx, void* Detour)
	{
		auto BigC = T::StaticClass();
		for (int i = 0; i < UObject::GObjects->Num(); ++i)
		{
			auto Obj = UObject::GObjects->GetByIndex(i);
			if (Obj)
			{
				if (Obj->IsDefaultObject())
				{
					if (Obj->IsA(BigC))
					{
						SwapVTableEntry(Obj->VTable, Idx, Detour, 0);
					}
				}
			}
		}
	}

	static bool PatchBytes(void* address, const void* bytes, size_t size)
	{
		if (!address || !bytes || size == 0)
			return false;

		DWORD oldProtect{};
		if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect))
			return false;

		std::memcpy(address, bytes, size);
		FlushInstructionCache(GetCurrentProcess(), address, size);

		DWORD temp{};
		VirtualProtect(address, size, oldProtect, &temp);
		return true;
	}

	static bool PatchByte(void* address, uint8_t value)
	{
		return PatchBytes(address, &value, sizeof(value));
	}

	static bool PatchByte(uintptr_t address, uint8_t value)
	{
		return PatchByte(reinterpret_cast<void*>(address), value);
	}

	static bool PatchByteChecked(uintptr_t address, uint8_t expected, uint8_t value)
	{
		auto ptr = reinterpret_cast<uint8_t*>(address);
		if (!ptr || *ptr != expected)
			return false;

		return PatchByte(address, value);
	}

	static bool NopBytes(void* address, size_t count)
	{
		if (!address || count == 0)
			return false;

		std::vector<uint8_t> nops(count, 0x90);
		return PatchBytes(address, nops.data(), nops.size());
	}

	static bool NopBytes(uintptr_t address, size_t count)
	{
		return NopBytes(reinterpret_cast<void*>(address), count);
	}

	static bool PatchCall(uintptr_t callSite, void* newTarget)
	{
		if (!callSite || !newTarget)
			return false;

		int32_t relOffset = (int32_t)((uintptr_t)newTarget - (callSite + 5));

		uint8_t patch[5];
		patch[0] = 0xE8;
		memcpy(&patch[1], &relOffset, sizeof(relOffset));

		return PatchBytes(reinterpret_cast<void*>(callSite), patch, sizeof(patch));
	}

	static bool PatchCallFar(uintptr_t callSite, void* newTarget)
	{
		uintptr_t base = callSite & ~0xFFFFull;
		void* trampoline = nullptr;

		for (uintptr_t addr = base; addr > base - 0x80000000ull; addr -= 0x10000)
		{
			trampoline = VirtualAlloc((void*)addr, 16, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (trampoline) break;
		}

		if (!trampoline)
			return false;

		uint8_t jmp[] = {
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};
		memcpy(&jmp[6], &newTarget, sizeof(newTarget));
		memcpy(trampoline, jmp, sizeof(jmp));

		return PatchCall(callSite, trampoline);
	}
};

inline void PatchAllNetModes(uintptr_t AttemptDeriveFromURL)
{
	Memcury::PE::Address add{ nullptr };

	const auto sizeOfImage = Memcury::PE::GetNTHeaders()->OptionalHeader.SizeOfImage;
	const auto scanBytes = reinterpret_cast<std::uint8_t*>(Memcury::PE::GetModuleBase());

	for (auto i = 0ul; i < sizeOfImage - 5; ++i)
	{
		if (scanBytes[i] == 0xE8 || scanBytes[i] == 0xE9)
		{
			if (Memcury::PE::Address(&scanBytes[i]).RelativeOffset(1).GetAs<void*>() == (void*)AttemptDeriveFromURL)
			{
				add = Memcury::PE::Address(&scanBytes[i]);

				// scan for the read of World->NetDriver

				for (auto j = 0; j > -0x100000; j--) // so we find everything. no func is actually 1mb
				{
					if ((scanBytes[i + j] & 0xF8) == 0x48 && ((scanBytes[i + j + 1] & 0xFC) == 0x80 || (scanBytes[i + j + 1] & 0xF8) == 0x38)
						&& (scanBytes[i + j + 2] & 0xF0) < 0xC0 && scanBytes[i + j + 2] != 0x65 && scanBytes[i + j + 2] != 0xBB && scanBytes[i + j + 3] == 0x48
						&& ((scanBytes[i + j + 1] & 0xFC) != 0x80 || scanBytes[i + j + 4] == 0x0))
					{
						// now, scan for if (NetDriver) return NM_Client;

						bool found = false;
						for (auto k = 4; k < 0x104; k++)
						{
							if (scanBytes[i + j + k] == 0x75)
							{
								auto Scuffness = __int64(&scanBytes[i + j + k + 5]);

								if (*(uint32_t*)Scuffness != 0x108 && (scanBytes[i + j + k + 4] != 0xC || scanBytes[i + j + k + 5] != 0xB) && scanBytes[i + j + k + 4] != 0x09)
									continue;

								Memory::Patch<uint16_t>(__int64(&scanBytes[i + j + k]), 0x9090);
								if ((scanBytes[i + j + 1] & 0xF8) == 0x38)
									Memory::Patch<uint32_t>(__int64(&scanBytes[i + j]), 0x90909090);
								else if ((scanBytes[i + j + 1] & 0xFC) == 0x80)
								{
									DWORD og;
									VirtualProtect(&scanBytes[i + j], 5, PAGE_EXECUTE_READWRITE, &og);
									*(uint32*)(&scanBytes[i + j]) = 0x90909090;
									*(uint8*)(&scanBytes[i + j + 4]) = 0x90;
									VirtualProtect(&scanBytes[i + j], 5, og, &og);
								}
								FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j], 5);
								FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j + k], 2);
								found = true;
								break;
							}
							else if (scanBytes[i + j + k] == 0x74)
							{
								auto Scuffness = __int64(&scanBytes[i + j + k]);
								Scuffness = (Scuffness + 2) + *(int8_t*)(Scuffness + 1);

								if (*(uint32_t*)(Scuffness + 3) != 0x108 && (*(uint8_t*)(Scuffness + 2) != 0xC || *(uint8_t*)(Scuffness + 3) != 0xB)
									&& *(uint8_t*)(Scuffness + 2) != 0x09)
									continue;

								Memory::Patch<uint8_t>(__int64(&scanBytes[i + j + k]), 0xeb);
								if ((scanBytes[i + j + 1] & 0xF8) == 0x38)
									Memory::Patch<uint32_t>(__int64(&scanBytes[i + j]), 0x90909090);
								else if ((scanBytes[i + j + 1] & 0xFC) == 0x80)
								{
									DWORD og;
									VirtualProtect(&scanBytes[i + j], 5, PAGE_EXECUTE_READWRITE, &og);
									*(uint32*)(&scanBytes[i + j]) = 0x90909090;
									*(uint8*)(&scanBytes[i + j + 4]) = 0x90;
									VirtualProtect(&scanBytes[i + j], 5, og, &og);
								}
								FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j], 5);
								FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j + k], 1);
								found = true;
								break;
							}
							else if (scanBytes[i + j + k] == 0x0F && scanBytes[i + j + k + 1] == 0x85)
							{
								auto Scuffness = __int64(&scanBytes[i + j + k + 9]);

								if (*(uint32_t*)Scuffness != 0x108 && (scanBytes[i + j + k + 8] != 0xC || scanBytes[i + j + k + 9] != 0xB) && scanBytes[i + j + k + 8] != 0x09)
									continue;

								DWORD og;
								VirtualProtect(&scanBytes[i + j + k], 6, PAGE_EXECUTE_READWRITE, &og);
								*(uint32*)(&scanBytes[i + j + k]) = 0x90909090;
								*(uint16*)(&scanBytes[i + j + k + 4]) = 0x9090;
								VirtualProtect(&scanBytes[i + j + k], 6, og, &og);
								if ((scanBytes[i + j + 1] & 0xF8) == 0x38)
									Memory::Patch<uint32_t>(__int64(&scanBytes[i + j]), 0x90909090);
								else if ((scanBytes[i + j + 1] & 0xFC) == 0x80)
								{
									DWORD og;
									VirtualProtect(&scanBytes[i + j], 5, PAGE_EXECUTE_READWRITE, &og);
									*(uint32*)(&scanBytes[i + j]) = 0x90909090;
									*(uint8*)(&scanBytes[i + j + 4]) = 0x90;
									VirtualProtect(&scanBytes[i + j], 5, og, &og);
								}
								FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j], 5);
								FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j + k], 6);
								found = true;
								break;
							}
							else if (scanBytes[i + j + k] == 0x0F && scanBytes[i + j + k + 1] == 0x84)
							{
								auto Scuffness = __int64(&scanBytes[i + j + k]);
								Scuffness = (Scuffness + 6) + *(int32_t*)(Scuffness + 2);

								if (*(uint32_t*)(Scuffness + 3) != 0x108 && (*(uint8_t*)(Scuffness + 2) != 0xC || *(uint8_t*)(Scuffness + 3) != 0xB)
									&& *(uint8_t*)(Scuffness + 2) != 0x09)
									continue;

								Memory::Patch<uint16_t>(__int64(&scanBytes[i + j + k]), 0xe990);
								if ((scanBytes[i + j + 1] & 0xF8) == 0x38)
									Memory::Patch<uint32_t>(__int64(&scanBytes[i + j]), 0x90909090);
								else if ((scanBytes[i + j + 1] & 0xFC) == 0x80)
								{
									DWORD og;
									VirtualProtect(&scanBytes[i + j], 5, PAGE_EXECUTE_READWRITE, &og);
									*(uint32*)(&scanBytes[i + j]) = 0x90909090;
									*(uint8*)(&scanBytes[i + j + 4]) = 0x90;
									VirtualProtect(&scanBytes[i + j], 5, og, &og);
								}
								FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j], 5);
								FlushInstructionCache(GetCurrentProcess(), &scanBytes[i + j + k], 2);
								found = true;
								break;
							}
						}
						if (found)
							break;
					}
				}
			}
		}
	}
}

static void RetNull() {}

static int True() {
	return 1;
}

static int False() {
	return 0;
}

#endif //PCH_H