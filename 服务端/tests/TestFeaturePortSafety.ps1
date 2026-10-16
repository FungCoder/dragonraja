$ErrorActionPreference = 'Stop'
$root = Split-Path (Split-Path $PSScriptRoot)
$output = Join-Path $root '服务端/runtime/feature-port-20261015/tests'
New-Item -ItemType Directory -Force -Path $output | Out-Null
$encoding = [Text.Encoding]::GetEncoding(28591)

function Get-ProductionSection($path, $startMarker, $endMarker) {
    $text = $encoding.GetString([IO.File]::ReadAllBytes((Join-Path $root $path)))
    $start = $text.IndexOf($startMarker, [StringComparison]::Ordinal)
    if ($start -lt 0) { throw "Missing section start in $path" }
    $end = $text.IndexOf($endMarker, $start, [StringComparison]::Ordinal)
    if ($end -le $start) { throw "Missing section end in $path" }
    return $text.Substring($start, $end - $start)
}

$production = Get-ProductionSection '服务端/Mapserver/HigherLayers/Menuserver.cpp' `
    'ItemAttr *GetItemByPOS( int cn, POS item )' `
    'extern void SendCMD_REQUEST_CHECK_WARNO'
$production += Get-ProductionSection '服务端/Mapserver/HigherLayers/QuestFunction.cpp' `
    'void CQuestInMap::GiveRequitalByCn' 'list<int> CQuestInMap::GetRoundUser'
[IO.File]::WriteAllBytes((Join-Path $output 'FeaturePortSafetyProduction.inc'),
    $encoding.GetBytes($production))

$adapterSource = [IO.File]::ReadAllText((Join-Path $root '服务端/Client/Debug/AutoPlayActions.cpp'))
$adapterStart = $adapterSource.IndexOf('namespace DragonAutoPlayClient')
if ($adapterStart -lt 0) { throw 'Missing auto play adapter namespace' }
$adapterProduction = $adapterSource.Substring($adapterStart)
$adapterProduction += Get-ProductionSection '服务端/Client/Debug/Skill.cpp' `
    'namespace DragonAutoPlayClient' 'typedef struct tagDETECTTILE'
[IO.File]::WriteAllBytes((Join-Path $output 'AutoPlayActionsProduction.inc'),
    $encoding.GetBytes($adapterProduction))
$vcvars = 'C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Auxiliary/Build/vcvars64.bat'
if (!(Test-Path -LiteralPath $vcvars)) { throw 'Visual Studio C++ build tools not found' }
$commands = @('@echo off', "call `"$vcvars`" >nul", 'if errorlevel 1 exit /b 1')
foreach ($test in @('FeaturePortSafetyTest', 'GameplaySafetyTest', 'AutoPlayActionsTest')) {
    $source = Join-Path $PSScriptRoot "$test.cpp"
    $executable = Join-Path $output "$test.exe"
    $object = Join-Path $output "$test.obj"
    $commands += "cl /nologo /EHsc /MT /Od /std:c++17 /wd4819 /I`"$output`" /Fe:`"$executable`" /Fo:`"$object`" `"$source`""
    $commands += 'if errorlevel 1 exit /b 1'
    $commands += "`"$executable`""
    $commands += 'if errorlevel 1 exit /b 1'
}
$commands += 'exit /b 0'
$batch = Join-Path $output 'RunSafetyTests.cmd'
[IO.File]::WriteAllLines($batch, (@('@echo off', 'chcp 65001 >nul') + $commands),
    [Text.UTF8Encoding]::new($false))
& cmd.exe /c $batch
if ($LASTEXITCODE -ne 0) { throw "Feature port safety tests failed: $LASTEXITCODE" }
