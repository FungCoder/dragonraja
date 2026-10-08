#include <windows.h>
#include <dbghelp.h>
#include <cstdio>
#include <cstring>
#include <string>

// Read only the exception location from a local dump; do not print memory contents.
int wmain(int argc, wchar_t** argv)
{
    if (argc != 3) { std::fprintf(stderr, "Usage: InspectMapCrash dump executable\n"); return 2; }
    HANDLE file = CreateFileW(argv[1], GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) { std::fprintf(stderr, "Cannot open dump\n"); return 1; }
    HANDLE mapping = CreateFileMappingW(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
    void* view = mapping ? MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0) : nullptr;
    int result = 1;
    HANDLE self = GetCurrentProcess();
    bool symbols = false;
    do {
        if (!view) { std::fprintf(stderr, "Cannot map dump\n"); break; }
        MINIDUMP_EXCEPTION_STREAM* exception = nullptr;
        MINIDUMP_MODULE_LIST* modules = nullptr;
        ULONG bytes = 0;
        if (!MiniDumpReadDumpStream(view, ExceptionStream, nullptr,
                reinterpret_cast<void**>(&exception), &bytes) || bytes < sizeof(*exception)) {
            std::fprintf(stderr, "No exception stream\n"); break;
        }
        if (!MiniDumpReadDumpStream(view, ModuleListStream, nullptr,
                reinterpret_cast<void**>(&modules), &bytes) || bytes < sizeof(ULONG) ||
            modules->NumberOfModules > (bytes - sizeof(ULONG)) / sizeof(MINIDUMP_MODULE)) {
            std::fprintf(stderr, "Invalid module stream\n"); break;
        }
        const auto& fault = exception->ExceptionRecord;
        std::printf("Exception=%08lX address=%llX thread=%lu\n",
            fault.ExceptionCode, fault.ExceptionAddress, exception->ThreadId);
        if (fault.ExceptionCode == EXCEPTION_ACCESS_VIOLATION && fault.NumberParameters >= 2)
            std::printf("Access operation=%llu address=%llX\n",
                fault.ExceptionInformation[0], fault.ExceptionInformation[1]);
        std::wstring search(argv[2]);
        search = search.substr(0, search.find_last_of(L"\\/"));
        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_EXACT_SYMBOLS);
        symbols = SymInitializeW(self, search.c_str(), FALSE) != FALSE;
        if (!symbols) { std::fprintf(stderr, "Symbol initialization failed\n"); break; }
        for (ULONG i = 0; i < modules->NumberOfModules; ++i) {
            const auto& module = modules->Modules[i];
            if (fault.ExceptionAddress < module.BaseOfImage ||
                fault.ExceptionAddress - module.BaseOfImage >= module.SizeOfImage) continue;
            std::printf("Module base=%llX RVA=%llX\n", module.BaseOfImage,
                fault.ExceptionAddress - module.BaseOfImage);
            if (!SymLoadModuleExW(self, nullptr, argv[2], nullptr, module.BaseOfImage,
                    module.SizeOfImage, nullptr, 0)) {
                std::fprintf(stderr, "Cannot load executable symbols\n"); break;
            }
            alignas(SYMBOL_INFO) unsigned char storage[sizeof(SYMBOL_INFO) + 1024] = {};
            auto info = reinterpret_cast<SYMBOL_INFO*>(storage);
            info->SizeOfStruct = sizeof(SYMBOL_INFO); info->MaxNameLen = 1024;
            DWORD64 displacement = 0;
            if (SymFromAddr(self, fault.ExceptionAddress, &displacement, info))
                std::printf("Function=%s +0x%llX\n", info->Name, displacement);
            IMAGEHLP_LINE64 line = {}; line.SizeOfStruct = sizeof(line);
            DWORD lineDisplacement = 0;
            if (SymGetLineFromAddr64(self, fault.ExceptionAddress, &lineDisplacement, &line))
                std::printf("Source=%s:%lu\n", line.FileName, line.LineNumber);
            result = 0;
            break;
        }
    } while (false);
    if (symbols) SymCleanup(self);
    if (view) UnmapViewOfFile(view);
    if (mapping) CloseHandle(mapping);
    CloseHandle(file);
    return result;
}
