// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
// Updated for VS2022 - MFC removed

#if !defined(AFX_STDAFX_H__CAE74589_5189_11D3_B887_004F4C04C077__INCLUDED_)
#define AFX_STDAFX_H__CAE74589_5189_11D3_B887_004F4C04C077__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN	// Exclude winsock.h from windows.h

// MFC headers removed - not needed for this project
//#include <afxwin.h>         // MFC core and standard components
//#include <afxext.h>         // MFC extensions
//#ifndef _AFX_NO_AFXCMN_SUPPORT
//#include <afxcmn.h>			// MFC support for Windows Common Controls
//#endif // _AFX_NO_AFXCMN_SUPPORT

// Order matters: winsock2.h MUST be included before windows.h
// to avoid winsock.h being included via windows.h
#include <winsock2.h>
#include <windows.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__CAE74589_5189_11D3_B887_004F4C04C077__INCLUDED_)
