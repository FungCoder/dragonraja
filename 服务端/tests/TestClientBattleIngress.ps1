$ErrorActionPreference = 'Stop'
$root = Split-Path (Split-Path $PSScriptRoot)
$output = Join-Path $root '服务端/runtime/combat-safety-20261016/tests'
New-Item -ItemType Directory -Force -Path $output | Out-Null
$byteEncoding = [Text.Encoding]::GetEncoding(28591)
$sourcePath = Join-Path $root '服务端/Mapserver/HigherLayers/DragonServer.cpp'
$source = $byteEncoding.GetString([IO.File]::ReadAllBytes($sourcePath))
$start = $source.IndexOf('int HandleUserCommand(int cn, t_packet* packet)', [StringComparison]::Ordinal)
if ($start -lt 0) { throw 'Missing user battle dispatch function' }
$end = $source.IndexOf('int HandleCommand( int cn, t_packet *packet)', $start, [StringComparison]::Ordinal)
if ($end -le $start) { throw 'Missing dispatch section boundary' }
[IO.File]::WriteAllBytes((Join-Path $output 'ClientBattleIngressProduction.inc'),
    $byteEncoding.GetBytes($source.Substring($start, $end - $start)))
$vcvars = 'C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Auxiliary/Build/vcvars64.bat'
if (!(Test-Path -LiteralPath $vcvars)) { throw 'C++ build tools not found' }
$executable = Join-Path $output 'ClientBattleIngressDispatchTest.exe'
$object = Join-Path $output 'ClientBattleIngressDispatchTest.obj'
$testSource = Join-Path $PSScriptRoot 'ClientBattleIngressDispatchTest.cpp'
$commands = @('@echo off', 'chcp 65001 >nul', "call `"$vcvars`" >nul", 'if errorlevel 1 exit /b 1',
    "cl /nologo /EHsc /MT /Od /std:c++17 /wd4819 /I`"$output`" /Fe:`"$executable`" /Fo:`"$object`" `"$testSource`"",
    'if errorlevel 1 exit /b 1', "`"$executable`"", 'exit /b %errorlevel%')
$batch = Join-Path $output 'RunBattleIngressTests.cmd'
[IO.File]::WriteAllLines($batch, $commands, [Text.UTF8Encoding]::new($false))
& cmd.exe /c $batch
if ($LASTEXITCODE -ne 0) { throw "Battle ingress tests failed: $LASTEXITCODE" }
