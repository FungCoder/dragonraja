// =====================================================================
// network_guid.h
// 4DyuchiNET 的 CLSID / IID 定义 (从 AgentServer 复制)
// =====================================================================
//
// CLSID_4DyuchiNET = {11C02A88-8BF9-4863-A7DE-1BF66D60CBA3}
// IID_4DyuchiNET   = {D41BD0F8-07BF-4dbc-8AD3-A3524CC43E5E}
//
// 在本 DLL 内,DEFINE_GUID 必须在 INITGUID 宏已定义的 TU 中生效(主 DLL 文件)。
// 其它 TU 仅 extern 常量即可。
// =====================================================================

#pragma once

#ifndef _NETWORK_GUID_H_INCLUDED
#define _NETWORK_GUID_H_INCLUDED

#include <guiddef.h>

// {11C02A88-8BF9-4863-A7DE-1BF66D60CBA3}
extern "C" const GUID CLSID_4DyuchiNET;

// {D41BD0F8-07BF-4dbc-8AD3-A3524CC43E5E}
extern "C" const GUID IID_4DyuchiNET;

#endif // _NETWORK_GUID_H_INCLUDED
