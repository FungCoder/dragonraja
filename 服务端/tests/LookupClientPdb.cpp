#include <windows.h>
#include <objbase.h>
#include <dia2.h>
#include <cstdio>
#include <cstdlib>
#include <cwchar>

int wmain(int argc, wchar_t** argv)
{
    if (argc != 4) {
        std::fwprintf(stderr, L"Usage: LookupClientPdb.exe msdia140.dll client.pdb RVA\n");
        return 2;
    }
    const bool lookupName = wcsncmp(argv[3], L"--name=", 7) == 0;
    const unsigned long rva = lookupName ? 0 : std::wcstoul(argv[3], nullptr, 0);
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    HMODULE library = LoadLibraryW(argv[1]);
    if (!library) { std::printf("DIA DLL unavailable: %lu\n", GetLastError()); return 2; }
    using GetFactory = HRESULT (STDAPICALLTYPE*)(REFCLSID, REFIID, void**);
    auto getFactory = reinterpret_cast<GetFactory>(GetProcAddress(library, "DllGetClassObject"));
    IClassFactory* factory = nullptr;
    IDiaDataSource* source = nullptr;
    IDiaSession* session = nullptr;
    HRESULT result = getFactory ? getFactory(CLSID_DiaSource, IID_IClassFactory,
        reinterpret_cast<void**>(&factory)) : E_FAIL;
    if (SUCCEEDED(result)) result = factory->CreateInstance(nullptr, IID_IDiaDataSource,
        reinterpret_cast<void**>(&source));
    if (SUCCEEDED(result)) result = source->loadDataFromPdb(argv[2]);
    if (SUCCEEDED(result)) result = source->openSession(&session);
    if (FAILED(result)) {
        std::printf("DIA initialization failed: 0x%08lX\n", result);
    } else {
        if (lookupName) {
            IDiaSymbol* global = nullptr;
            IDiaEnumSymbols* matches = nullptr;
            if (SUCCEEDED(session->get_globalScope(&global)) && global &&
                SUCCEEDED(session->findChildren(global, SymTagData, argv[3] + 7, 0, &matches)) && matches) {
                IDiaSymbol* found = nullptr;
                ULONG count = 0;
                while (SUCCEEDED(matches->Next(1, &found, &count)) && count && found) {
                    DWORD symbolRva = 0;
                    ULONGLONG bytes = 0;
                    found->get_relativeVirtualAddress(&symbolRva);
                    found->get_length(&bytes);
                    std::wprintf(L"Data=%ls RVA=0x%lX Size=%llu\n", argv[3] + 7,
                        symbolRva, static_cast<unsigned long long>(bytes));
                    found->Release();
                }
                matches->Release();
            }
            if (global) global->Release();
            session->Release(); source->Release(); factory->Release();
            FreeLibrary(library); CoUninitialize();
            return 0;
        }
        const enum SymTagEnum tags[] = {SymTagFunction, SymTagPublicSymbol};
        for (enum SymTagEnum tag : tags) {
            IDiaSymbol* symbol = nullptr;
            if (SUCCEEDED(session->findSymbolByRVA(rva, tag, &symbol)) && symbol) {
                BSTR name = nullptr;
                if (SUCCEEDED(symbol->get_name(&name)) && name) {
                    std::wprintf(L"Symbol=%ls RVA=0x%lX\n", name, rva);
                    SysFreeString(name);
                }
                symbol->Release();
                break;
            }
        }
        IDiaEnumLineNumbers* lines = nullptr;
        if (SUCCEEDED(session->findLinesByRVA(rva, 1, &lines)) && lines) {
            IDiaLineNumber* line = nullptr;
            ULONG fetched = 0;
            if (SUCCEEDED(lines->Next(1, &line, &fetched)) && fetched && line) {
                DWORD number = 0;
                line->get_lineNumber(&number);
                IDiaSourceFile* file = nullptr;
                if (SUCCEEDED(line->get_sourceFile(&file)) && file) {
                    BSTR path = nullptr;
                    if (SUCCEEDED(file->get_fileName(&path)) && path) {
                        std::wprintf(L"File=%ls Line=%lu\n", path, number);
                        SysFreeString(path);
                    }
                    file->Release();
                }
                line->Release();
            }
            lines->Release();
        }
    }
    if (session) session->Release();
    if (source) source->Release();
    if (factory) factory->Release();
    FreeLibrary(library);
    CoUninitialize();
    return FAILED(result) ? 1 : 0;
}
