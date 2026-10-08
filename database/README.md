# 三个 MySQL 数据库的开源安装基线

版本日期：2026-10-08。三个数据库安装文件已在MySQL 8.0.12独立实例完成恢复验证，用于新环境安装；完整游戏内容仍需实际运行验收。

恢复核对结果：主库213张表、总库43张表、日志库26张表，共282张表；主库100个过程、总库3个过程。125张基础数据表共38,930行，其余表为空。`map_info`保留103条地图目录记录，实际可启动地图仍取决于资源和配置。

## 文件说明

| 文件 | 内容 |
|---|---|
| `dragonraja.sql` | 主库表结构、存储过程及物品、技能、NPC、地图、任务等基础游戏数据 |
| `dragonraja_total.sql` | 总库表结构和存储过程 |
| `dragonraja_log.sql` | 日志库表结构 |
| `baseline-manifest.json` | 各文件字节数/SHA256，以及每张表导入后的行数和数据策略 |

清单中的`game-baseline`表示基础数据表，`schema-only`表示安装后为空的业务表。新安装没有玩家账号，需要按下文自行创建本地测试账号。

初始配置如下，部署时按实际环境设置：

- 地图地址为127.0.0.1，服务路径为`.`，在线人数和状态为0。
- 建筑、国王及投票处于初始状态。
- 存储过程DEFINER使用导入用户`CURRENT_USER`，应由本地数据库管理员导入。

安装后还需配置MySQL连接用户、服务INI和实际地图资源；部分经验模板及地图玩法仍待核验。

## 首次安装

### 1. 准备与核验

使用全新 MySQL 8 测试实例。文件使用 utf8mb4，包含 utf8mb4_0900_ai_ci 排序规则；MariaDB、MySQL 5.7以及其他版本未验证。Windows客户端使用 x64 ODBC ANSI驱动，详见 [主说明](../README.md)。

在源码根目录核验清单：

```powershell
$databaseRoot = Join-Path $PWD 'database'
$baseline = Get-Content (Join-Path $databaseRoot 'baseline-manifest.json') -Raw | ConvertFrom-Json
foreach ($file in $baseline.Files) {
    $actual = Get-FileHash (Join-Path $databaseRoot $file.File) -Algorithm SHA256
    if ($actual.Hash -ne $file.SHA256) { throw "数据库文件校验失败：$($file.File)" }
}
```

### 2. 导入三个库

**SQL包含建库、DROP TABLE和DROP PROCEDURE语句，会替换同名对象。只在首次安装的空实例执行；已有正式库不能直接覆盖。** 安装使用这里的三个MySQL文件。

先交互保存本地连接凭据；替换管理员用户名，密码由工具提示输入，不写进命令行：

```powershell
mysql_config_editor.exe set --login-path=dragonraja-install --host=127.0.0.1 --user=数据库管理员 --password
```

在源码根目录执行，每条都检查退出码：

```powershell
cmd.exe /d /c 'mysql.exe --login-path=dragonraja-install --default-character-set=utf8mb4 < database\dragonraja.sql'
if ($LASTEXITCODE -ne 0) { throw '主库导入失败' }
cmd.exe /d /c 'mysql.exe --login-path=dragonraja-install --default-character-set=utf8mb4 < database\dragonraja_total.sql'
if ($LASTEXITCODE -ne 0) { throw '总库导入失败' }
cmd.exe /d /c 'mysql.exe --login-path=dragonraja-install --default-character-set=utf8mb4 < database\dragonraja_log.sql'
if ($LASTEXITCODE -ne 0) { throw '日志库导入失败' }
```

Linux等环境可使用同样的 mysql 客户端参数和shell输入重定向，游戏服务端本身仍是Windows工程。跨操作系统表名大小写和文件系统行为未完整验收。

### 3. 检查导入结果

使用mysql客户端查看：

```sql
SELECT TABLE_SCHEMA,COUNT(*) AS table_count
FROM information_schema.TABLES
WHERE TABLE_SCHEMA IN ('dragonraja','dragonraja_total','dragonraja_log')
GROUP BY TABLE_SCHEMA;
SELECT ROUTINE_SCHEMA,COUNT(*) AS routine_count
FROM information_schema.ROUTINES
WHERE ROUTINE_SCHEMA IN ('dragonraja','dragonraja_total','dragonraja_log')
GROUP BY ROUTINE_SCHEMA;
SELECT COUNT(*) AS map_count FROM dragonraja.map_info;
SELECT COUNT(*) AS public_accounts FROM dragonraja_total.chr_log_info;
SELECT COUNT(*) AS public_characters FROM dragonraja.chr_info;
SHOW CREATE PROCEDURE dragonraja_total.up_get_user_info2;
```

账号和角色应为0行。各表预期行数见清单。登录过程的结果列必须按 `id_index、passwd、d_kyulje、d_eday、timeremain` 返回，不能随意调整列顺序。

### 4. 配置数据库用户和本地测试账号

MySQL用户用于服务端连接；游戏账号用于客户端登录，两者不同。由管理员创建专用MySQL用户并按实际调用授予三个库的业务权限，限制来源主机，然后在未跟踪的本地INI中配置。不要将安装管理员长期作为游戏运行账号。

公开基线不提供游戏账号。当前 `DBDemon/Pay.cpp` 对总库 `chr_log_info.passwd` 做直接字符串比对，属于旧协议，不能将此字段直接换成哈希后期待原客户端继续登录。请为隔离测试使用独立口令，不复用个人或生产密码，不将填好的账号脚本公开发布。

通过数据库客户端创建测试账号：先用 `SHOW COLUMNS FROM dragonraja_total.chr_log_info` 确认字段；选择唯一 `login_id` 和非重复的数值 `id_index`，填入本地测试口令 `passwd`，设置有效的 `d_kyulje`、`d_eday`、`timeremain`。该基线字段多为text，登录过程进行数值/时间转换，日期须可解析且符合所选收费/免费模式。不要复制空密码或公开共享账号。

在**刚导入且总库账号表为空的本地测试实例**中，可用64位Windows PowerShell和ODBC参数绑定创建第一个账号；不会把密码写进SQL文本。账号和口令按当前协议限制为1～20个可打印ASCII字符，不要以空格结尾。以下不适用于已有账号库或多人并发注册：

```powershell
$databaseCredential = Get-Credential -Message '输入本地测试实例的数据库管理员'
$playerCredential = Get-Credential -Message '输入自行选择的游戏测试账号和独立口令'
$playerPassword = $playerCredential.GetNetworkCredential().Password
if ($playerCredential.UserName -notmatch '^[A-Za-z0-9_]{1,20}$' -or
    $playerPassword -notmatch '^[\x21-\x7E]{1,20}$') { throw '账号或口令不符合测试协议限制' }
$builder = [Data.Odbc.OdbcConnectionStringBuilder]::new()
$builder.Driver = 'MySQL ODBC 9.7 ANSI Driver'
$builder['Server'] = '127.0.0.1'
$builder['Port'] = 3306
$builder['Database'] = 'dragonraja_total'
$builder['UID'] = $databaseCredential.UserName
$builder['PWD'] = $databaseCredential.GetNetworkCredential().Password
$connection = [Data.Odbc.OdbcConnection]::new($builder.ConnectionString)
$command = $null
try {
    $connection.Open()
    $command = $connection.CreateCommand()
    $command.CommandText = 'SELECT COUNT(*) FROM chr_log_info'
    if ([long]$command.ExecuteScalar() -ne 0) { throw '仅允许初始化空账号库' }
    $command.CommandText = 'INSERT INTO chr_log_info (id_index,login_id,passwd,d_kyulje,d_eday,timeremain) VALUES (?, ?, ?, ?, ?, ?)'
    foreach ($value in @('1', $playerCredential.UserName, $playerPassword, '0',
        (Get-Date).AddYears(1).ToString('yyyy-MM-dd HH:mm:ss'), '0')) {
        $parameter = $command.Parameters.Add(('value' + $command.Parameters.Count), [Data.Odbc.OdbcType]::VarChar, 40)
        $parameter.Value = $value
    }
    if ($command.ExecuteNonQuery() -ne 1) { throw '本地测试账号创建失败' }
} finally {
    if ($command) { $command.Dispose() }
    $connection.Dispose()
    $playerPassword = $null
    $builder.Clear()
}
```

地址、端口和驱动名称按你的测试环境修改。这里只提供账号创建方法，本次数据库恢复验证未执行玩家创角或登录游戏，也未提供注册网站。生产注册系统需要单独设计并审查安全边界。

当前登录源码在鉴权通过后仍检查本地免费/收费配置；免费测试模式也不会绕过账号密码验证。角色应在客户端正常创建，不复制旧玩家的 chr_info 行。游戏内创角、进图和保存仍需合法客户端资源及完整服务配置，数据库导入不能替代它们。

### 5. 开服前完成路由与资源配置

公开地图地址为回环地址，服务路径为通用值；按你的部署环境统一设置服务INI和数据库中的IP、端口、地图目录及路由。核验资源齐全，再按 [服务端使用说明](../README.md#服务端使用说明) 启动少量地图并完成游戏内验收。

## 恢复验证范围

三个SQL在独立MySQL实例中从空库导入，逐表核对表数量和行数，并验证总库登录过程对不存在的账号返回空结果。

此验证覆盖数据库可恢复性和部分调用契约，不替代玩家登录、跨图、战斗或交易等游戏内验收。
