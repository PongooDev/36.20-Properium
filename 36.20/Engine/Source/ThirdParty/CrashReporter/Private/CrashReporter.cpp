#include "pch.h"
#include "../Public/CrashReporter.h"

void FreezeOtherThreads()
{
    auto thrHandle = GetCurrentThread();
    auto currentThr = GetThreadId(thrHandle);
    auto currentPrc = GetProcessIdOfThread(thrHandle);
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (h != INVALID_HANDLE_VALUE)
    {
        THREADENTRY32 te;
        te.dwSize = sizeof(te);
        if (Thread32First(h, &te))
        {
            do
            {
                if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
                {
                    if (te.th32ThreadID != currentThr && te.th32OwnerProcessID == currentPrc)
                    {
                        auto thr = OpenThread(THREAD_ALL_ACCESS, false, te.th32ThreadID);

                        if (thr != INVALID_HANDLE_VALUE)
                        {
                            SuspendThread(thr);
                            CloseHandle(thr);
                        }
                    }
                }
                te.dwSize = sizeof(te);
            } while (Thread32Next(h, &te));
        }
        CloseHandle(h);
    }
}

DWORD FormatNtStatus(NTSTATUS nsCode, TCHAR** ppszMessage)
{
    HMODULE ntdll = LoadLibraryA("ntdll.dll");

    if (ntdll == NULL)
        return 0;

    DWORD outLen = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE, ntdll, RtlNtStatusToDosError(nsCode),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)ppszMessage, 0, NULL);

    FreeLibrary(ntdll);

    return outLen;
}

LONG WINAPI CoreUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
    if ((ExceptionInfo->ExceptionRecord->ExceptionCode & 0x80000000) == 0 || (ExceptionInfo->ExceptionRecord->ExceptionCode & 0x30000000) != 0)
        return EXCEPTION_CONTINUE_SEARCH;

    FreezeOtherThreads();

    char symName[1024 * sizeof(TCHAR)];
    char symStorage[sizeof(IMAGEHLP_SYMBOL64) + sizeof(symName)];
    auto sym = (IMAGEHLP_SYMBOL64*)symStorage;

    auto currentPrc = GetCurrentProcess();
    DWORD64 displacement = 0;

    SymCleanup(currentPrc);

    auto InitResult = SymInitialize(currentPrc, nullptr, true);

    if (!InitResult)
        Log("[CrashReporter] Failed to initialize symbol finder!");

    std::stringstream reportStream;

    reportStream << "[CrashReporter] Caught unhandled exception (Code: ";

    switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
    {
#define NAMED_EX(x)                                                                                                                                                                \
    case x:                                                                                                                                                                        \
        reportStream << #x;                                                                                                                                                        \
        break;

        NAMED_EX(EXCEPTION_ACCESS_VIOLATION);
        NAMED_EX(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
        NAMED_EX(EXCEPTION_BREAKPOINT);
        NAMED_EX(EXCEPTION_DATATYPE_MISALIGNMENT);
        NAMED_EX(EXCEPTION_FLT_DENORMAL_OPERAND);
        NAMED_EX(EXCEPTION_FLT_DIVIDE_BY_ZERO);
        NAMED_EX(EXCEPTION_FLT_INEXACT_RESULT);
        NAMED_EX(EXCEPTION_FLT_INVALID_OPERATION);
        NAMED_EX(EXCEPTION_FLT_OVERFLOW);
        NAMED_EX(EXCEPTION_FLT_STACK_CHECK);
        NAMED_EX(EXCEPTION_FLT_UNDERFLOW);
        NAMED_EX(EXCEPTION_ILLEGAL_INSTRUCTION);
        NAMED_EX(EXCEPTION_INT_DIVIDE_BY_ZERO);
        NAMED_EX(EXCEPTION_INT_OVERFLOW);
        NAMED_EX(EXCEPTION_INVALID_DISPOSITION);
        NAMED_EX(EXCEPTION_NONCONTINUABLE_EXCEPTION);
        NAMED_EX(EXCEPTION_PRIV_INSTRUCTION);
        NAMED_EX(EXCEPTION_SINGLE_STEP);
        NAMED_EX(EXCEPTION_STACK_OVERFLOW);

#undef NAMED_EX
    default:
        reportStream << std::format("0x{:08X}", static_cast<unsigned int>(ExceptionInfo->ExceptionRecord->ExceptionCode));
    }
    reportStream << ")";

    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_IN_PAGE_ERROR)
    {
        switch (ExceptionInfo->ExceptionRecord->ExceptionInformation[0])
        {
        case 0:
            reportStream << "\n- Trying to read ";
            break;
        case 1:
            reportStream << "\n- Trying to write ";
            break;
        case 8:
            reportStream << "\n- Trying to execute ";
            break;
        }
        if (ExceptionInfo->ExceptionRecord->ExceptionInformation[1] == 0xFFFFF78000000900) // fn anti debug check ig
            return EXCEPTION_CONTINUE_SEARCH;

        reportStream << std::format("0x{:016X}", static_cast<uint64_t>(ExceptionInfo->ExceptionRecord->ExceptionInformation[1]));
    }
    reportStream << "\n\n";

    CONTEXT ctx = *ExceptionInfo->ContextRecord;

    if (!ctx.Rip && ctx.Rsp)
    {
        reportStream << "0x0000000000000000: [null function pointer call, caller follows]\n";
        ctx.Rip = *reinterpret_cast<DWORD64*>(ctx.Rsp);
        ctx.Rsp += 8;
    }

    for (int frame = 0; frame < 64 && ctx.Rip; frame++)
    {
        DWORD64 pc = ctx.Rip;

        reportStream << std::format("0x{:016X}", static_cast<uint64_t>(pc));

        auto imageBase = SymGetModuleBase64(currentPrc, pc);
        if (imageBase)
        {
            char path[1024];
            GetModuleFileNameA(HMODULE(imageBase), path, 1024);

            auto filteredPath = strrchr(path, '\\');

            reportStream << " (" << (filteredPath ? filteredPath + 1 : path) << "+"
                         << std::format("0x{:X}", static_cast<uint64_t>(pc - imageBase)) << ")";
        }
        reportStream << ": ";

        sym->SizeOfStruct = sizeof(symStorage);
        sym->MaxNameLength = sizeof(symName);

        BOOL SymResult = SymGetSymFromAddr64(currentPrc, (ULONG64)pc, &displacement, sym);
        if (SymResult == false || imageBase == ImageBase)
            reportStream << "[unknown]\n";
        else
        {
            UnDecorateSymbolName(sym->Name, (PSTR)symName, sizeof(symName), UNDNAME_COMPLETE);

            reportStream << sym->Name << "()";
            IMAGEHLP_LINE64 ImageHelpLine = { 0 };
            ImageHelpLine.SizeOfStruct = sizeof(ImageHelpLine);
            if (SymGetLineFromAddr64(currentPrc, (ULONG64)pc, (::DWORD*)&displacement, &ImageHelpLine))
            {
                auto filteredName = strrchr(ImageHelpLine.FileName, '\\');
                reportStream << " [" << (filteredName ? filteredName + 1 : ImageHelpLine.FileName) << ":" << ImageHelpLine.LineNumber << "]";
            }
            reportStream << "\n";
        }

        DWORD64 unwindBase = 0;
        PRUNTIME_FUNCTION funcEntry = RtlLookupFunctionEntry(pc, &unwindBase, nullptr);
        if (funcEntry)
        {
            PVOID handlerData = nullptr;
            DWORD64 establisherFrame = 0;
            RtlVirtualUnwind(UNW_FLAG_NHANDLER, unwindBase, pc, funcEntry, &ctx, &handlerData, &establisherFrame, nullptr);
        }
        else
        {
            if (!ctx.Rsp)
                break;
            ctx.Rip = *reinterpret_cast<DWORD64*>(ctx.Rsp);
            ctx.Rsp += 8;
        }
    }
    auto reportStr = reportStream.str();
    Log(reportStr);

    Memcury::Util::CopyToClipboard(reportStr);
    SymCleanup(currentPrc);
    Sleep(3000);
    // while (true) {}
    TerminateProcess(GetCurrentProcess(), ExceptionInfo->ExceptionRecord->ExceptionCode);
    // ExitProcess(ExceptionInfo->ExceptionRecord->ExceptionCode);
    return EXCEPTION_CONTINUE_EXECUTION;
}

void FCrashReporter::Register()
{
    AddVectoredExceptionHandler(0, CoreUnhandledExceptionFilter);
}