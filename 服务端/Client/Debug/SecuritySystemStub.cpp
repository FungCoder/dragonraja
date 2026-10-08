#include "stdafx.h"
#include "SecuritySystem.h"

CSecuritySystem::CSecuritySystem() : m_hSafeWnd(NULL)
{
    m_szTempName[0] = 0;
}

CSecuritySystem::~CSecuritySystem()
{
    Disconnect();
}

int CSecuritySystem::Connect(HWND)
{
    return 1;
}

void CSecuritySystem::Disconnect()
{
}

bool CSecuritySystem::CheckFileName(const char* pFileName)
{
    char module_path[MAX_PATH] = {0,};
    char file_name[_MAX_FNAME + _MAX_EXT] = {0,};
    char name[_MAX_FNAME] = {0,};
    char extension[_MAX_EXT] = {0,};

    GetModuleFileName(NULL, module_path, sizeof(module_path));
    _splitpath(module_path, NULL, NULL, name, extension);
    sprintf_s(file_name, "%s%s", name, extension);
    return _stricmp(file_name, pFileName) == 0;
}

HWND CSecuritySystem::GetDragonHwnd()
{
    return FindWindow("DRAGONRAJA_CLASS", NULL);
}

bool CSecuritySystem::MakeExeFile(int, const char*)
{
    return false;
}
