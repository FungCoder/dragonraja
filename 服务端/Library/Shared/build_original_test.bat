@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" 1>nul 2>&1
cd /d "%~dp0"
cl /nologo /EHsc /MT /wd4819 /Fe:hsel_original_test.exe hsel_original_test.cpp ^
  /link /LIBPATH:"..\..\Client\Library" HSEL.lib /NODEFAULTLIB:libc.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib uxtheme.lib comdlg32.lib winspool.lib odbc32.lib odbccp32.lib windowscodecs.lib dxguid.lib ^
  /ALTERNATENAME:?Initial@CHSEL_STREAM@@QAEHUHSEL_INITIAL@@@Z=?Initial@CHSEL_STREAM@@QAE_HUHSEL_INITIAL@@@Z ^
  /ALTERNATENAME:?Encrypt@CHSEL_STREAM@@QAE_NPADH@Z=?Encrypt@CHSEL_STREAM@@QAE_NPAD_H@Z ^
  /ALTERNATENAME:?Decrypt@CHSEL_STREAM@@QAE_NPADH@Z=?Decrypt@CHSEL_STREAM@@QAE_NPAD_H@Z ^
  /ALTERNATENAME:?GetCRCConvertInt@CHSEL_STREAM@@QBEHXZ=?GetCRCConvertInt@CHSEL_STREAM@@QBE_HXZ
