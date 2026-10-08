// =====================================================================
// 4DyuchiNETStub.cpp
// 龙族(Dragon Raja) 4DyuchiNET COM 存根实现
// =====================================================================
//
// 用途:
//   4DyuchiNET 是 2001 年韩国商业第三方游戏网络库，已无法获取。
//   本存根提供最小可用的 COM 实现，让 AgentServer/MapServer/ProxyServer
//   能够启动并进入主循环。
//
// 警告:
//   这只是占位实现，没有真实的网络 I/O。
//   - SendTo* 方法返回 TRUE 但不发送任何数据
//   - ConnectTo*/Start* 方法返回 TRUE 但不监听端口
//   - 服务器之间无法通信
//
// 如需真正运行游戏服务器，请:
//   1. 寻找原始 4DyuchiNET.dll 并 regsvr32 注册（覆盖本存根）
//   2. 或将代码中的 CoCreateInstance 替换为真实的网络库（如 Boost.Asio）
//
// 编译: Visual Studio 2022, v143, x64 Release
// 注册: regsvr32 4DyuchiNETStub.dll
// 卸载: regsvr32 /u 4DyuchiNETStub.dll
// =====================================================================

#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <objbase.h>
#include <olectl.h>

// 强制 DEFINE_GUID 在此 TU 中生成 GUID 定义
#define INITGUID
#include <guiddef.h>

// 必须在 include <objbase.h> 后定义 GUID (使用 initguid.h 强制生成定义)
// {11C02A88-8BF9-4863-A7DE-1BF66D60CBA3}
DEFINE_GUID(CLSID_4DyuchiNET,
    0x11c02a88, 0x8bf9, 0x4863, 0xa7, 0xde, 0x1b, 0xf6, 0x6d, 0x60, 0xcb, 0xa3);

// {D41BD0F8-07BF-4dbc-8AD3-A3524CC43E5E}
DEFINE_GUID(IID_4DyuchiNET,
    0xd41bd0f8, 0x07bf, 0x4dbc, 0x8a, 0xd3, 0xa3, 0x52, 0x4c, 0xc4, 0x3e, 0x5e);

// =====================================================================
// 函数指针类型定义 (与 AgentServer/typedef.h 一致)
// =====================================================================
typedef void (__stdcall *ACCEPTFUNC)(DWORD);
typedef void (__stdcall *RECVFUNC)(DWORD dwConnectionIndex, char* pMsg, DWORD dwLength);
typedef void (__stdcall *CONNECTSUCCESSFUNC)(DWORD dwConnectionIndex, void* pVoid);
typedef void (__stdcall *CONNECTFAILFUNC)(void* pVoid);
typedef void (__stdcall *DISCONNECTFUNC)(DWORD dwConnectionIndex);
typedef void (__stdcall *EVENTFUNC)(DWORD dwEventIndex);

struct CUSTOM_EVENT
{
    DWORD       dwPeriodicTime;
    EVENTFUNC   pEventFunc;
};

struct DESC_NETWORK
{
    DWORD           dwMaxUserNum;
    DWORD           dwMaxServerNum;
    RECVFUNC        OnRecvFromUserTCP;
    RECVFUNC        OnRecvFromServerTCP;
    ACCEPTFUNC      OnAcceptUser;
    ACCEPTFUNC      OnAcceptServer;
    DISCONNECTFUNC  OnDisconnectUser;
    DISCONNECTFUNC  OnDisconnectServer;
    DWORD           dwServerMaxTransferSize;
    DWORD           dwUserMaxTransferSize;
    DWORD           dwServerBufferSizePerConnection;
    DWORD           dwUserBufferSizePerConnection;
    DWORD           dwMainMsgQueMaxBufferSize;
    DWORD           dwConnectNumAtSameTime;
    DWORD           dwFlag;
    DWORD           dwCustomDefineEventNum;
    CUSTOM_EVENT*   pEvent;
};

struct PACKET_LIST
{
    char*           pMsg;
    DWORD           dwLen;
    PACKET_LIST*    pNext;
    DWORD           dwFlag;
};

// =====================================================================
// I4DyuchiNET 接口
// =====================================================================
interface I4DyuchiNET : public IUnknown
{
    virtual BOOL            __stdcall CreateNetwork(DESC_NETWORK* desc, DWORD dwUserAcceptInterval, DWORD dwServerAcceptInterval) = 0;
    virtual void            __stdcall BreakMainThread() = 0;
    virtual void            __stdcall ResumeMainThread() = 0;
    virtual void            __stdcall SetUserInfo(DWORD dwConnectionIndex, void* user) = 0;
    virtual void*           __stdcall GetUserInfo(DWORD dwConnectionIndex) = 0;
    virtual void            __stdcall SetServerInfo(DWORD dwConnectionIndex, void* server) = 0;
    virtual void*           __stdcall GetServerInfo(DWORD dwConnectionIndex) = 0;
    virtual sockaddr_in*    __stdcall GetServerAddress(DWORD dwConnectionIndex) = 0;
    virtual sockaddr_in*    __stdcall GetUserAddress(DWORD dwConnectionIndex) = 0;
    virtual BOOL            __stdcall GetServerAddress(DWORD dwConnectionIndex, char* pIP, WORD* pwPort) = 0;
    virtual BOOL            __stdcall GetUserAddress(DWORD dwConnectionIndex, char* pIP, WORD* pwPort) = 0;
    virtual BOOL            __stdcall SendToServer(DWORD dwConnectionIndex, char* msg, DWORD length, DWORD flag) = 0;
    virtual BOOL            __stdcall SendToUser(DWORD dwConnectionIndex, char* msg, DWORD length, DWORD flag) = 0;
    virtual void            __stdcall CompulsiveDisconnectServer(DWORD dwConnectionIndex) = 0;
    virtual void            __stdcall CompulsiveDisconnectUser(DWORD dwConnectionIndex) = 0;
    virtual int             __stdcall GetServerMaxTransferRecvSize() = 0;
    virtual int             __stdcall GetServerMaxTransferSendSize() = 0;
    virtual int             __stdcall GetUserMaxTransferRecvSize() = 0;
    virtual int             __stdcall GetUserMaxTransferSendSize() = 0;
    virtual void            __stdcall BroadcastServer(char* pMsg, DWORD len, DWORD flag) = 0;
    virtual void            __stdcall BroadcastUser(char* pMsg, DWORD len, DWORD flag) = 0;
    virtual DWORD           __stdcall GetConnectedServerNum() = 0;
    virtual DWORD           __stdcall GetConnectedUserNum() = 0;
    virtual WORD            __stdcall GetBindedPortServerSide() = 0;
    virtual WORD            __stdcall GetBindedPortUserSide() = 0;
    virtual BOOL            __stdcall ConnectToServerWithUserSide(char* szIP, WORD port, CONNECTSUCCESSFUNC pSucc, CONNECTFAILFUNC pFail, void* pExt) = 0;
    virtual BOOL            __stdcall ConnectToServerWithServerSide(char* szIP, WORD port, CONNECTSUCCESSFUNC pSucc, CONNECTFAILFUNC pFail, void* pExt) = 0;
    virtual BOOL            __stdcall StartServerWithUserSide(char* ip, WORD port) = 0;
    virtual BOOL            __stdcall StartServerWithServerSide(char* ip, WORD port) = 0;
    virtual HANDLE          __stdcall GetCustomEventHandle(DWORD index) = 0;
    virtual BOOL            __stdcall PauseTimer(DWORD dwCustomEventIndex) = 0;
    virtual BOOL            __stdcall ResumeTimer(DWORD dwCustomEventIndex) = 0;
    virtual BOOL            __stdcall SendToServer(DWORD dwConnectionIndex, WSABUF* pBuf, DWORD dwNum, DWORD flag) = 0;
    virtual BOOL            __stdcall SendToUser(DWORD dwConnectionIndex, WSABUF* pBuf, DWORD dwNum, DWORD flag) = 0;
    virtual BOOL            __stdcall SendToServer(DWORD dwConnectionIndex, PACKET_LIST* pList, DWORD flag) = 0;
    virtual BOOL            __stdcall SendToUser(DWORD dwConnectionIndex, PACKET_LIST* pList, DWORD flag) = 0;
};

// =====================================================================
// C4DyuchiNETStub 实现
// =====================================================================
class C4DyuchiNETStub : public I4DyuchiNET
{
private:
    LONG    m_refCount;
    HANDLE  m_hEvents[8];      // 最大 8 个自定义事件 (覆盖 AgentServer 用到的 0/1/2/3)
    BOOL    m_bTimerPaused[8];
    DWORD   m_dwPeriodicTime[8];
    EVENTFUNC m_pEventFunc[8];

public:
    C4DyuchiNETStub() : m_refCount(1)
    {
        for (int i = 0; i < 8; ++i)
        {
            m_hEvents[i] = CreateEventW(NULL, FALSE, FALSE, NULL);
            m_bTimerPaused[i] = TRUE;
            m_dwPeriodicTime[i] = 0;
            m_pEventFunc[i] = NULL;
        }
    }

    virtual ~C4DyuchiNETStub()
    {
        for (int i = 0; i < 8; ++i)
        {
            if (m_hEvents[i]) CloseHandle(m_hEvents[i]);
        }
    }

    // IUnknown
    virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv)
    {
        if (riid == IID_IUnknown || riid == IID_4DyuchiNET)
        {
            *ppv = static_cast<I4DyuchiNET*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    virtual ULONG __stdcall AddRef()
    {
        return (ULONG)InterlockedIncrement(&m_refCount);
    }

    virtual ULONG __stdcall Release()
    {
        LONG ref = InterlockedDecrement(&m_refCount);
        if (ref == 0)
        {
            delete this;
            return 0;
        }
        return (ULONG)ref;
    }

    // I4DyuchiNET
    virtual BOOL __stdcall CreateNetwork(DESC_NETWORK* desc, DWORD, DWORD)
    {
        if (desc && desc->pEvent && desc->dwCustomDefineEventNum > 0)
        {
            DWORD n = (desc->dwCustomDefineEventNum < 8) ? desc->dwCustomDefineEventNum : 8;
            for (DWORD i = 0; i < n; ++i)
            {
                m_dwPeriodicTime[i] = desc->pEvent[i].dwPeriodicTime;
                m_pEventFunc[i] = desc->pEvent[i].pEventFunc;
            }
        }
        return TRUE;
    }

    virtual void __stdcall BreakMainThread() {}
    virtual void __stdcall ResumeMainThread() {}

    virtual void __stdcall SetUserInfo(DWORD, void*) {}
    virtual void* __stdcall GetUserInfo(DWORD) { return NULL; }
    virtual void __stdcall SetServerInfo(DWORD, void*) {}
    virtual void* __stdcall GetServerInfo(DWORD) { return NULL; }

    virtual sockaddr_in* __stdcall GetServerAddress(DWORD) { return NULL; }
    virtual sockaddr_in* __stdcall GetUserAddress(DWORD) { return NULL; }
    virtual BOOL __stdcall GetServerAddress(DWORD, char*, WORD*) { return FALSE; }
    virtual BOOL __stdcall GetUserAddress(DWORD, char*, WORD*) { return FALSE; }

    virtual BOOL __stdcall SendToServer(DWORD, char*, DWORD, DWORD) { return TRUE; }
    virtual BOOL __stdcall SendToUser(DWORD, char*, DWORD, DWORD) { return TRUE; }

    virtual void __stdcall CompulsiveDisconnectServer(DWORD) {}
    virtual void __stdcall CompulsiveDisconnectUser(DWORD) {}

    virtual int __stdcall GetServerMaxTransferRecvSize() { return 65000; }
    virtual int __stdcall GetServerMaxTransferSendSize() { return 65000; }
    virtual int __stdcall GetUserMaxTransferRecvSize() { return 8192; }
    virtual int __stdcall GetUserMaxTransferSendSize() { return 8192; }

    virtual void __stdcall BroadcastServer(char*, DWORD, DWORD) {}
    virtual void __stdcall BroadcastUser(char*, DWORD, DWORD) {}

    virtual DWORD __stdcall GetConnectedServerNum() { return 0; }
    virtual DWORD __stdcall GetConnectedUserNum() { return 0; }

    virtual WORD __stdcall GetBindedPortServerSide() { return 0; }
    virtual WORD __stdcall GetBindedPortUserSide() { return 0; }

    virtual BOOL __stdcall ConnectToServerWithUserSide(char*, WORD, CONNECTSUCCESSFUNC, CONNECTFAILFUNC, void*) { return TRUE; }
    virtual BOOL __stdcall ConnectToServerWithServerSide(char*, WORD, CONNECTSUCCESSFUNC, CONNECTFAILFUNC, void*) { return TRUE; }
    virtual BOOL __stdcall StartServerWithUserSide(char*, WORD) { return TRUE; }
    virtual BOOL __stdcall StartServerWithServerSide(char*, WORD) { return TRUE; }

    virtual HANDLE __stdcall GetCustomEventHandle(DWORD index)
    {
        if (index < 8 && m_hEvents[index]) return m_hEvents[index];
        return NULL;
    }

    virtual BOOL __stdcall PauseTimer(DWORD index)
    {
        if (index < 8) { m_bTimerPaused[index] = TRUE; return TRUE; }
        return FALSE;
    }

    virtual BOOL __stdcall ResumeTimer(DWORD index)
    {
        if (index < 8) { m_bTimerPaused[index] = FALSE; return TRUE; }
        return FALSE;
    }

    virtual BOOL __stdcall SendToServer(DWORD, WSABUF*, DWORD, DWORD) { return TRUE; }
    virtual BOOL __stdcall SendToUser(DWORD, WSABUF*, DWORD, DWORD) { return TRUE; }
    virtual BOOL __stdcall SendToServer(DWORD, PACKET_LIST*, DWORD) { return TRUE; }
    virtual BOOL __stdcall SendToUser(DWORD, PACKET_LIST*, DWORD) { return TRUE; }
};

// =====================================================================
// 类工厂
// =====================================================================
class C4DyuchiNETFactory : public IClassFactory
{
private:
    LONG m_refCount;

public:
    C4DyuchiNETFactory() : m_refCount(1) {}

    virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv)
    {
        if (riid == IID_IUnknown || riid == IID_IClassFactory)
        {
            *ppv = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    virtual ULONG __stdcall AddRef()
    {
        return (ULONG)InterlockedIncrement(&m_refCount);
    }

    virtual ULONG __stdcall Release()
    {
        LONG ref = InterlockedDecrement(&m_refCount);
        if (ref == 0) { delete this; return 0; }
        return (ULONG)ref;
    }

    virtual HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv)
    {
        if (pUnkOuter != NULL) return CLASS_E_NOAGGREGATION;
        C4DyuchiNETStub* pObj = new C4DyuchiNETStub();
        if (!pObj) return E_OUTOFMEMORY;
        HRESULT hr = pObj->QueryInterface(riid, ppv);
        pObj->Release();
        return hr;
    }

    virtual HRESULT __stdcall LockServer(BOOL) { return S_OK; }
};

// =====================================================================
// DLL 入口
// =====================================================================
static LONG g_serverLocks = 0;
static HINSTANCE g_hInstance = NULL;

extern "C" HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    if (!IsEqualCLSID(rclsid, CLSID_4DyuchiNET)) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    C4DyuchiNETFactory* pFactory = new C4DyuchiNETFactory();
    if (!pFactory) return E_OUTOFMEMORY;
    HRESULT hr = pFactory->QueryInterface(riid, ppv);
    pFactory->Release();
    return hr;
}

extern "C" HRESULT __stdcall DllCanUnloadNow()
{
    return (g_serverLocks == 0) ? S_OK : S_FALSE;
}

static HRESULT WriteRegistryString(HKEY hRootKey, LPCWSTR sub, LPCWSTR value, LPCWSTR data)
{
    HKEY hKey;
    LONG r = RegCreateKeyExW(hRootKey, sub, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    if (r != ERROR_SUCCESS) return E_FAIL;
    DWORD bytes = (DWORD)((wcslen(data) + 1) * sizeof(WCHAR));
    r = RegSetValueExW(hKey, value, 0, REG_SZ, (const BYTE*)data, bytes);
    RegCloseKey(hKey);
    return (r == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

extern "C" HRESULT __stdcall DllRegisterServer()
{
    WCHAR path[MAX_PATH];
    GetModuleFileNameW(g_hInstance, path, MAX_PATH);

    static const WCHAR CLSID_STR[] = L"{11C02A88-8BF9-4863-A7DE-1BF66D60CBA3}";
    WCHAR keyPath[128];

    // HKCR\CLSID\{CLSID} = "4DyuchiNET Stub"
    wsprintfW(keyPath, L"CLSID\\%s", CLSID_STR);
    HRESULT hr = WriteRegistryString(HKEY_CLASSES_ROOT, keyPath, NULL, L"4DyuchiNET Stub");
    if (FAILED(hr)) return hr;

    // HKCR\CLSID\{CLSID}\InprocServer32 = "dll path"
    wsprintfW(keyPath, L"CLSID\\%s\\InprocServer32", CLSID_STR);
    hr = WriteRegistryString(HKEY_CLASSES_ROOT, keyPath, NULL, path);
    if (FAILED(hr)) return hr;

    // ThreadingModel = "Apartment"
    WriteRegistryString(HKEY_CLASSES_ROOT, keyPath, L"ThreadingModel", L"Apartment");

    return S_OK;
}

extern "C" HRESULT __stdcall DllUnregisterServer()
{
    static const WCHAR CLSID_STR[] = L"{11C02A88-8BF9-4863-A7DE-1BF66D60CBA3}";
    WCHAR keyPath[128];

    wsprintfW(keyPath, L"CLSID\\%s\\InprocServer32", CLSID_STR);
    RegDeleteKeyW(HKEY_CLASSES_ROOT, keyPath);

    wsprintfW(keyPath, L"CLSID\\%s", CLSID_STR);
    RegDeleteKeyW(HKEY_CLASSES_ROOT, keyPath);

    return S_OK;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReason, LPVOID)
{
    if (ulReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hModule;
        DisableThreadLibraryCalls(hModule);
    }
    return TRUE;
}
