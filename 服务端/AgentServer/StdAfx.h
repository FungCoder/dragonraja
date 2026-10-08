#pragma once

// winsock2.h 必须在 windows.h 之前包含，否则 winsock.h 会先被包含导致函数重定义
#include <winsock2.h>
#include <ole2.h>
#include <initguid.h>
#include <windows.h>
// x64 库 - 直接编译源代码而不使用 32 位 .lib
#include "../Library/Shared/HSEL.h"
#include "../Library/Shared/Shared.h"

// ASSERT 宏替代 MFC 定义
#ifndef ASSERT
#ifdef _DEBUG
#include <crtdbg.h>
#define ASSERT(expr) _ASSERT(expr)
#else
#define ASSERT(expr) ((void)0)
#endif
#endif

#if !defined(_DLLEXPORT_)
	// If _DLLEXPORT_ is NOT defined then the default is to import.
	#if defined(__cplusplus)
		#define DLLENTRY extern "C" __declspec(dllimport)
	#else
		#define DLLENTRY extern __declspec(dllimport)
	#endif

	#define STDENTRY DLLENTRY HRESULT WINAPI
	#define STDENTRY_(type) DLLENTRY type WINAPI

	// Here is the list of server APIs offered by the DLL (using the
	// appropriate entry API declaration macros just #defined above).

	STDENTRY DllRegisterServer(void);
	STDENTRY DllUnregisterServer(void);
#else  // _DLLEXPORT_
	// Else if _DLLEXPORT_ is indeed defined then we've been told to export.
	#if defined(__cplusplus)
		#define DLLENTRY extern "C" __declspec(dllexport)
	#else
		#define DLLENTRY __declspec(dllexport)
	#endif

	#define STDENTRY DLLENTRY HRESULT WINAPI
	#define STDENTRY_(type) DLLENTRY type WINAPI
#endif // _DLLEXPORT_

#define GUID_SIZE 128
#define MAX_STRING_LENGTH 256
typedef void**	PPVOID;
//010909 lsw
#include "LocalizingMgr.h"//021007 lsw