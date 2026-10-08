// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Windows headers (order matters: winsock2.h before windows.h)
#include <winsock2.h>
#include <windows.h>
#include <ole2.h>
#include <initguid.h>
#include <stddef.h>     // NULL definition
#include <crtdbg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mmsystem.h>
#include <direct.h>

// ODBC headers
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

// ODBC compatibility: map old SDWORD to SQLLEN for modern Windows SDK
// In legacy MSSQL-era code, SQLGetData used SDWORD* as last param
// In modern ODBC, it uses SQLLEN*
// sqltypes.h typedef's SDWORD as long (32-bit), but ODBC APIs now expect
// SQLLEN* (__int64 on x64). Use #define to shadow the typedef so all
// subsequent SDWORD tokens resolve to SQLLEN at the preprocessor level.
#define SDWORD SQLLEN

// Library dependencies (source compiled directly, not linking .lib)
#include "../Library/Shared/HSEL.h"
#include "../Library/Shared/Shared.h"

// ASSERT/VERIFY macros (MFC compatibility)
#ifndef ASSERT
#define ASSERT(f)          ((void)0)
#endif
#ifndef VERIFY
#define VERIFY(f)          ((void)(f))
#endif
#ifndef DEBUG_NEW
#define DEBUG_NEW          new
#endif

#define rand()	rand_()		// 020707 YGI
extern int rand_();

#include "HigherLayers/DefaultHeader.h"

#include "GameSystem.h"



