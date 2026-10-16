$ErrorActionPreference = 'Stop'
$root = Split-Path (Split-Path $PSScriptRoot)
$output = Join-Path $root '服务端/runtime/combat-shot-20261016/tests'
New-Item -ItemType Directory -Force -Path $output | Out-Null
$sourcePath = Join-Path $root '服务端/Mapserver/HigherLayers/CombatShotServer.cpp'
$source = [IO.File]::ReadAllText($sourcePath)
$start = $source.IndexOf('namespace', [StringComparison]::Ordinal)
if ($start -lt 0) { throw 'Missing production shot server namespace' }
[IO.File]::WriteAllText((Join-Path $output 'CombatShotServerProduction.inc'),
    $source.Substring($start), [Text.UTF8Encoding]::new($false))
$clientSource = [IO.File]::ReadAllText((Join-Path $root '服务端/Client/Debug/CombatShotClient.cpp'))
$clientNamespace = [regex]::Match($clientSource, '(?m)^namespace\s*\{')
if (!$clientNamespace.Success) { throw 'Missing production shot client namespace' }
[IO.File]::WriteAllText((Join-Path $output 'CombatShotClientProduction.inc'),
    $clientSource.Substring($clientNamespace.Index), [Text.UTF8Encoding]::new($false))
$legacyEncoding = [Text.Encoding]::GetEncoding(28591)
$battleSource = $legacyEncoding.GetString([IO.File]::ReadAllBytes(
    (Join-Path $root '服务端/Client/Debug/Battle.cpp')))
$generatorSource = ''
foreach ($markers in @(
    @('static int s_aAngle8[8]', 'static POINT s_aPoint[9]'),
    @('static int s_aOffset[8][5]', 'static POINT s_aCircle[29]'),
    @('void CBattle::EffectSpread(', 'void CBattle::EffectBreath('),
    @('void CBattle::AddEffect(', 'void CBattle::ClearCrash('))) {
    $sectionStart = $battleSource.IndexOf($markers[0], [StringComparison]::Ordinal)
    if ($sectionStart -lt 0) { throw "Missing generator start: $($markers[0])" }
    $sectionEnd = $battleSource.IndexOf($markers[1], $sectionStart, [StringComparison]::Ordinal)
    if ($sectionEnd -le $sectionStart) { throw "Missing generator end: $($markers[1])" }
    $generatorSource += $battleSource.Substring($sectionStart, $sectionEnd - $sectionStart)
}
[IO.File]::WriteAllBytes((Join-Path $output 'CombatBurstGeneratorProduction.inc'),
    $legacyEncoding.GetBytes($generatorSource))
$effectSource = $legacyEncoding.GetString([IO.File]::ReadAllBytes(
    (Join-Path $root '服务端/Client/Debug/Effect.cpp')))
$effectStart = $effectSource.IndexOf('static void InsertMagicContext(', [StringComparison]::Ordinal)
$effectEnd = $effectSource.IndexOf('void InsertEffect(', $effectStart, [StringComparison]::Ordinal)
if ($effectStart -lt 0 -or $effectEnd -le $effectStart) { throw 'Missing production effect context' }
[IO.File]::WriteAllBytes((Join-Path $output 'CombatShotEffectProduction.inc'),
    $legacyEncoding.GetBytes($effectSource.Substring($effectStart, $effectEnd - $effectStart)))
$combatSource = $legacyEncoding.GetString([IO.File]::ReadAllBytes(
    (Join-Path $root '服务端/Client/Debug/CombatSys.cpp')))
$rootNamespace = [regex]::Match($combatSource, '(?m)^namespace\s*\{')
if (!$rootNamespace.Success) { throw 'Missing production root namespace' }
$rootStart = $rootNamespace.Index
$rootEnd = $combatSource.IndexOf('extern int g_CurrCharStatus;', $rootStart, [StringComparison]::Ordinal)
if ($rootStart -lt 0 -or $rootEnd -le $rootStart) { throw 'Missing production root helper' }
[IO.File]::WriteAllBytes((Join-Path $output 'CombatShotRootProduction.inc'),
    $legacyEncoding.GetBytes($combatSource.Substring($rootStart, $rootEnd - $rootStart)))
$managerSource = $legacyEncoding.GetString([IO.File]::ReadAllBytes(
    (Join-Path $root '服务端/Mapserver/HigherLayers/BattleManager.cpp')))
$arrowStart = $managerSource.IndexOf('bool CBattleManager::PrepareMagicArrowShot(', [StringComparison]::Ordinal)
$arrowEnd = $managerSource.IndexOf('void CBattleManager::RecvMagicResult(t_magic_result_d*', $arrowStart, [StringComparison]::Ordinal)
if ($arrowStart -lt 0 -or $arrowEnd -le $arrowStart) { throw 'Missing magic-arrow authority handlers' }
[IO.File]::WriteAllBytes((Join-Path $output 'MagicArrowAuthorityProduction.inc'),
    $legacyEncoding.GetBytes($managerSource.Substring($arrowStart, $arrowEnd - $arrowStart)))
$strikeSource = $legacyEncoding.GetString([IO.File]::ReadAllBytes(
    (Join-Path $root '服务端/Client/Debug/StrikeSys.cpp')))
$strikeNamespace = [regex]::Match($strikeSource, '(?m)^namespace\s*\{')
$strikeEnd = $strikeSource.IndexOf('//////////////////////////////////////////////////////////////////////', $strikeNamespace.Index, [StringComparison]::Ordinal)
if (!$strikeNamespace.Success -or $strikeEnd -le $strikeNamespace.Index) { throw 'Missing actual projectile binding helper' }
[IO.File]::WriteAllBytes((Join-Path $output 'CombatShotStrikeProduction.inc'),
    $legacyEncoding.GetBytes($strikeSource.Substring($strikeNamespace.Index, $strikeEnd - $strikeNamespace.Index)))
$shootStart = $strikeSource.IndexOf('void CStrikeSys::Shoot(', [StringComparison]::Ordinal)
$shootEnd = $strikeSource.IndexOf('void CStrikeSys::Throw(', $shootStart, [StringComparison]::Ordinal)
if ($shootStart -lt 0 -or $shootEnd -le $shootStart) { throw 'Missing production Shoot method' }
[IO.File]::WriteAllBytes((Join-Path $output 'CombatShotShootProduction.inc'),
    $legacyEncoding.GetBytes($strikeSource.Substring($shootStart, $shootEnd - $shootStart)))
$vcvars = 'C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Auxiliary/Build/vcvars64.bat'
if (!(Test-Path -LiteralPath $vcvars)) { throw 'C++ build tools not found' }
$commands = @('@echo off', 'chcp 65001 >nul', "call `"$vcvars`" >nul", 'if errorlevel 1 exit /b 1')
foreach ($testName in @('CombatShotServerTest', 'CombatShotClientTest')) {
    $executable = Join-Path $output "$testName.exe"
    $object = Join-Path $output "$testName.obj"
    $testSource = Join-Path $PSScriptRoot "$testName.cpp"
    $commands += "cl /nologo /EHsc /MT /Od /std:c++17 /wd4819 /I`"$output`" /Fe:`"$executable`" /Fo:`"$object`" `"$testSource`""
    $commands += 'if errorlevel 1 exit /b 1'
    $commands += "`"$executable`""
    $commands += 'if errorlevel 1 exit /b 1'
}
$commands += 'exit /b 0'
$batch = Join-Path $output 'RunCombatShotServerTests.cmd'
[IO.File]::WriteAllLines($batch, $commands, [Text.UTF8Encoding]::new($false))
& cmd.exe /c $batch
if ($LASTEXITCODE -ne 0) { throw "Shot server tests failed: $LASTEXITCODE" }
