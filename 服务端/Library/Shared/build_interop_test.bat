@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" 1>nul 2>&1
cd /d "%~dp0"
cl /nologo /EHsc /MT /wd4819 /Fe:hsel_interop_test.exe hsel_interop_test.cpp HSEL.cpp
