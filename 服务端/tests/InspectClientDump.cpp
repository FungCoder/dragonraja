#include <windows.h>
#include <dbghelp.h>
#include <cstdio>
#include <cwchar>
#include <cstdlib>
#include <cstring>

bool ReadDumpWord(void* view, ULONG64 address, DWORD& value)
{
    PMINIDUMP_DIRECTORY directory = nullptr;
    void* stream = nullptr;
    ULONG length = 0;
    if (MiniDumpReadDumpStream(view, Memory64ListStream, &directory, &stream, &length)) {
        const auto* memories = static_cast<const MINIDUMP_MEMORY64_LIST*>(stream);
        ULONG64 fileOffset = memories->BaseRva;
        for (ULONG64 i = 0; i < memories->NumberOfMemoryRanges; ++i) {
            const auto& range = memories->MemoryRanges[i];
            if (address >= range.StartOfMemoryRange &&
                address - range.StartOfMemoryRange + sizeof(value) <= range.DataSize) {
                std::memcpy(&value, static_cast<const char*>(view) + fileOffset +
                    address - range.StartOfMemoryRange, sizeof(value));
                return true;
            }
            fileOffset += range.DataSize;
        }
    }
    if (MiniDumpReadDumpStream(view, MemoryListStream, &directory, &stream, &length)) {
        const auto* memories = static_cast<const MINIDUMP_MEMORY_LIST*>(stream);
        for (ULONG i = 0; i < memories->NumberOfMemoryRanges; ++i) {
            const auto& range = memories->MemoryRanges[i];
            if (address >= range.StartOfMemoryRange &&
                address - range.StartOfMemoryRange + sizeof(value) <= range.Memory.DataSize) {
                std::memcpy(&value, static_cast<const char*>(view) + range.Memory.Rva +
                    address - range.StartOfMemoryRange, sizeof(value));
                return true;
            }
        }
    }
    return false;
}

int wmain(int argc, wchar_t** argv)
{
    if (argc < 3) {
        std::fwprintf(stderr, L"Usage: InspectClientDump.exe dump.dmp client.exe [address...]\n");
        return 2;
    }
    HANDLE file = CreateFileW(argv[1], GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return 2;
    HANDLE mapping = CreateFileMappingW(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!mapping) { CloseHandle(file); return 2; }
    void* view = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    if (!view) { CloseHandle(mapping); CloseHandle(file); return 2; }
    PMINIDUMP_DIRECTORY directory = nullptr;
    void* stream = nullptr;
    ULONG length = 0;
    if (!MiniDumpReadDumpStream(view, ExceptionStream, &directory, &stream, &length) ||
        length < sizeof(MINIDUMP_EXCEPTION_STREAM)) {
        std::fprintf(stderr, "No exception stream in dump\n");
        UnmapViewOfFile(view); CloseHandle(mapping); CloseHandle(file);
        return 1;
    }
    const auto* exception = static_cast<const MINIDUMP_EXCEPTION_STREAM*>(stream);
    const DWORD64 address = exception->ExceptionRecord.ExceptionAddress;
    std::printf("Exception=0x%08lX Address=0x%llX Thread=%lu\n",
        exception->ExceptionRecord.ExceptionCode,
        static_cast<unsigned long long>(address), exception->ThreadId);
    if (exception->ExceptionRecord.NumberParameters >= 2)
        std::printf("AccessKind=%llu InvalidAddress=0x%llX\n",
            static_cast<unsigned long long>(exception->ExceptionRecord.ExceptionInformation[0]),
            static_cast<unsigned long long>(exception->ExceptionRecord.ExceptionInformation[1]));
    if (exception->ThreadContext.DataSize >= sizeof(WOW64_CONTEXT)) {
        const auto* registers = reinterpret_cast<const WOW64_CONTEXT*>(
            static_cast<const char*>(view) + exception->ThreadContext.Rva);
        std::printf("EIP=%08lX EAX=%08lX EBX=%08lX ECX=%08lX EDX=%08lX ESI=%08lX EDI=%08lX EBP=%08lX ESP=%08lX\n",
            registers->Eip, registers->Eax, registers->Ebx, registers->Ecx,
            registers->Edx, registers->Esi, registers->Edi, registers->Ebp, registers->Esp);
        DWORD iteration = 0, group = 0;
        if (ReadDumpWord(view, registers->Ebp - 8, iteration) &&
            ReadDumpWord(view, registers->Ebp - 12, group))
            std::printf("SkillIndex=%lu Inclusive=%lu\n", iteration, group);
    }
    for (int index = 3; index < argc; ++index) {
        const ULONG64 target = std::wcstoull(argv[index], nullptr, 0);
        DWORD value = 0;
        if (ReadDumpWord(view, target, value))
            std::printf("Memory[0x%llX]=0x%08lX (%lu)\n",
                static_cast<unsigned long long>(target), value, value);
        else
            std::printf("Memory[0x%llX] not captured\n",
                static_cast<unsigned long long>(target));
    }

    DWORD64 imageBase = 0;
    if (MiniDumpReadDumpStream(view, ModuleListStream, &directory, &stream, &length) &&
        length >= sizeof(ULONG)) {
        const auto* modules = static_cast<const MINIDUMP_MODULE_LIST*>(stream);
        for (ULONG index = 0; index < modules->NumberOfModules; ++index) {
            const MINIDUMP_MODULE& module = modules->Modules[index];
            if (address >= module.BaseOfImage &&
                address - module.BaseOfImage < module.SizeOfImage) {
                imageBase = module.BaseOfImage;
                const auto* name = reinterpret_cast<const MINIDUMP_STRING*>(
                    static_cast<const char*>(view) + module.ModuleNameRva);
                std::wprintf(L"FaultingModule=%.*ls\n",
                    static_cast<int>(name->Length / sizeof(wchar_t)), name->Buffer);
                std::printf("ModuleBase=0x%llX RVA=0x%llX Size=%lu\n",
                    static_cast<unsigned long long>(module.BaseOfImage),
                    static_cast<unsigned long long>(address - module.BaseOfImage),
                    module.SizeOfImage);
                break;
            }
        }
    }
    HANDLE process = GetCurrentProcess();
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS);
    if (SymInitializeW(process, nullptr, FALSE)) {
        wchar_t fullPath[MAX_PATH] = {};
        GetFullPathNameW(argv[2], MAX_PATH, fullPath, nullptr);
        const DWORD64 loaded = SymLoadModuleExW(process, nullptr, fullPath, nullptr,
            imageBase, 0, nullptr, 0);
        if (loaded) {
            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {};
            auto* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = MAX_SYM_NAME;
            DWORD64 displacement = 0;
            if (SymFromAddr(process, address, &displacement, symbol)) {
                std::printf("Symbol=%s+0x%llX\n", symbol->Name,
                    static_cast<unsigned long long>(displacement));
                IMAGEHLP_LINE64 line = {};
                line.SizeOfStruct = sizeof(line);
                DWORD lineOffset = 0;
                if (SymGetLineFromAddr64(process, address, &lineOffset, &line))
                    std::printf("SourceLine=%lu Offset=%lu\n", line.LineNumber, lineOffset);
            } else {
                std::printf("Symbol lookup failed: %lu\n", GetLastError());
            }
        } else {
            std::printf("Module symbol loading failed: %lu\n", GetLastError());
        }
        SymCleanup(process);
    }
    UnmapViewOfFile(view); CloseHandle(mapping); CloseHandle(file);
    return 0;
}
