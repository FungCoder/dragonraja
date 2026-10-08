
#if !defined(AFX_AGENTSERVER_H__F4B2DCF8_D77A_426D_8C68_DE978C3113E8__INCLUDED_)
#define AFX_AGENTSERVER_H__F4B2DCF8_D77A_426D_8C68_DE978C3113E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MFC removed - not needed for this project
//#include <afx.h>
#define WIN32_LEAN_AND_MEAN	// Exclude winsock.h from windows.h
// Order matters: winsock2.h MUST be included before windows.h
// to avoid winsock.h being included via windows.h
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <ole2.h>
#include <initguid.h>


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

#include <map>
#include <vector>
using namespace std;

#define GUID_SIZE 128
#define MAX_STRING_LENGTH 256
typedef void**	PPVOID;

#define MY_STRING	"Thank you all" // CSD-040120
#include "LocalizingMgr.h"//021007 lsw
#define COMMENT /##/

// ASSERT/VERIFY macro replacement for MFC
#ifndef ASSERT
#ifdef _DEBUG
#include <crtdbg.h>
#define ASSERT(expr) _ASSERTE(expr)
#else
#define ASSERT(expr) ((void)0)
#endif
#endif

#ifndef VERIFY
#ifdef _DEBUG
#define VERIFY(expr) _ASSERTE(expr)
#else
#define VERIFY(expr) ((expr))
#endif
#endif

#endif // !defined(AFX_AGENTSERVER_H__F4B2DCF8_D77A_426D_8C68_DE978C3113E8__INCLUDED_)
