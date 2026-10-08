// =====================================================================
// 4DyuchiNETAsio.cpp
// DLL 入口 + IClassFactory + DllXxx 导出
// =====================================================================

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// Asio 必须在 windows.h 之前包含
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/windows/object_handle.hpp>

#include <windows.h>
#include <winsock2.h>
#include <objbase.h>
#include <olectl.h>

// 强制 DEFINE_GUID 在此 TU 中生成 GUID 定义
#define INITGUID
#include <guiddef.h>

// GUID 定义 (与 AgentServer/network_guid.h 一致)
DEFINE_GUID(CLSID_4DyuchiNET,
    0x11c02a88, 0x8bf9, 0x4863, 0xa7, 0xde, 0x1b, 0xf6, 0x6d, 0x60, 0xcb, 0xa3);

DEFINE_GUID(IID_4DyuchiNET,
    0xd41bd0f8, 0x07bf, 0x4dbc, 0x8a, 0xd3, 0xa3, 0x52, 0x4c, 0xc4, 0x3e, 0x5e);

#include "../include/inetwork.h"
#include "../include/network_guid.h"
#include "4DyuchiNETAsioImpl.h"

// =====================================================================
// 类工厂
// =====================================================================
class C4DyuchiNETAsioFactory : public IClassFactory
{
private:
    LONG m_ref_count;

public:
    C4DyuchiNETAsioFactory() : m_ref_count(1) {}

    virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override
    {
        if (ppv == nullptr) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IClassFactory) {
            *ppv = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    virtual ULONG __stdcall AddRef() override
    {
        return static_cast<ULONG>(InterlockedIncrement(&m_ref_count));
    }

    virtual ULONG __stdcall Release() override
    {
        LONG ref = InterlockedDecrement(&m_ref_count);
        if (ref == 0) { delete this; return 0; }
        return static_cast<ULONG>(ref);
    }

    virtual HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) override
    {
        if (pUnkOuter != nullptr) return CLASS_E_NOAGGREGATION;
        asionet::C4DyuchiNETAsio* pObj = new asionet::C4DyuchiNETAsio();
        if (pObj == nullptr) return E_OUTOFMEMORY;
        HRESULT hr = pObj->QueryInterface(riid, ppv);
        pObj->Release();
        return hr;
    }

    virtual HRESULT __stdcall LockServer(BOOL) override { return S_OK; }
};

// =====================================================================
// 全局状态
// =====================================================================
static LONG      g_server_locks = 0;
static HINSTANCE g_h_instance   = NULL;

// =====================================================================
// 导出函数
// =====================================================================
extern "C" HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    if (ppv == nullptr) return E_POINTER;
    if (!IsEqualCLSID(rclsid, CLSID_4DyuchiNET)) {
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
    C4DyuchiNETAsioFactory* pFactory = new C4DyuchiNETAsioFactory();
    if (pFactory == nullptr) return E_OUTOFMEMORY;
    HRESULT hr = pFactory->QueryInterface(riid, ppv);
    pFactory->Release();
    return hr;
}

extern "C" HRESULT __stdcall DllCanUnloadNow()
{
    return (g_server_locks == 0) ? S_OK : S_FALSE;
}

static HRESULT WriteRegistryString(HKEY hRootKey, LPCWSTR sub, LPCWSTR value, LPCWSTR data)
{
    HKEY hKey;
    LONG r = RegCreateKeyExW(hRootKey, sub, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    if (r != ERROR_SUCCESS) return E_FAIL;
    DWORD bytes = static_cast<DWORD>((wcslen(data) + 1) * sizeof(WCHAR));
    r = RegSetValueExW(hKey, value, 0, REG_SZ, reinterpret_cast<const BYTE*>(data), bytes);
    RegCloseKey(hKey);
    return (r == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

extern "C" HRESULT __stdcall DllRegisterServer()
{
    WCHAR path[MAX_PATH];
    // 使用 GetModuleHandleExW 而非依赖 g_h_instance,更稳健
    HMODULE self = nullptr;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCWSTR>(&DllRegisterServer), &self) == 0) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (GetModuleFileNameW(self, path, MAX_PATH) == 0) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    static const WCHAR CLSID_STR[] = L"{11C02A88-8BF9-4863-A7DE-1BF66D60CBA3}";
    WCHAR keyPath[128];

    // Per-user COM registration keeps deployment compatible with standard
    // Windows accounts and avoids requiring administrative privileges.
    wsprintfW(keyPath, L"Software\\Classes\\CLSID\\%s", CLSID_STR);
    HRESULT hr = WriteRegistryString(HKEY_CURRENT_USER, keyPath, nullptr, L"4DyuchiNET Asio Implementation");
    if (FAILED(hr)) return hr;

    // HKCU\Software\Classes\CLSID\{CLSID}\InprocServer32 = "dll path"
    wsprintfW(keyPath, L"Software\\Classes\\CLSID\\%s\\InprocServer32", CLSID_STR);
    hr = WriteRegistryString(HKEY_CURRENT_USER, keyPath, nullptr, path);
    if (FAILED(hr)) return hr;

    // ThreadingModel = "Apartment"
    WriteRegistryString(HKEY_CURRENT_USER, keyPath, L"ThreadingModel", L"Apartment");

    return S_OK;
}

extern "C" HRESULT __stdcall DllUnregisterServer()
{
    static const WCHAR CLSID_STR[] = L"{11C02A88-8BF9-4863-A7DE-1BF66D60CBA3}";
    WCHAR keyPath[128];

    wsprintfW(keyPath, L"Software\\Classes\\CLSID\\%s\\InprocServer32", CLSID_STR);
    RegDeleteKeyW(HKEY_CURRENT_USER, keyPath);

    wsprintfW(keyPath, L"Software\\Classes\\CLSID\\%s", CLSID_STR);
    RegDeleteKeyW(HKEY_CURRENT_USER, keyPath);

    return S_OK;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReason, LPVOID)
{
    if (ulReason == DLL_PROCESS_ATTACH) {
        g_h_instance = hModule;
        DisableThreadLibraryCalls(hModule);
    }
    return TRUE;
}
