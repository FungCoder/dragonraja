# Dragon Raja Windows / MySQL

更新日期：2026-10-08。本仓库提供游戏客户端、服务端源码和MySQL数据库安装文件。

使用顺序：[完整操作流程](#从安装到进入游戏) → [数据库安装](database/README.md) → [源码编译](#源码编译说明) → [服务端配置启动](#服务端使用说明) → [客户端IP与登录](#客户端配置与进入游戏)。本仓库没有注册网站，也不包含完整游戏运行资源。

## 配套客户端下载

[下载配套客户端（123云盘）](https://1855115188.share.123pan.cn/123pan/UmzWvd-ejB13?pwd=kohh)

提取码：`kohh`。完整解压到英文路径后，双击`dragonraja.exe`启动游戏；连接其他服务器前，按[客户端配置](#客户端配置与进入游戏)修改`dragon.ini`中的IP和登录端口。

## 目录

| 目录 | 内容 |
|---|---|
| `database` | 当前三个MySQL库的导出SQL、逐表行数及SHA256清单 |
| `服务端/DBDemon` | 游戏数据库服务RajaDB |
| `服务端/AgentServer` | 登录和连接服务 |
| `服务端/Mapserver` | 经典地图服务及玩法代码 |
| `服务端/4DyuchiNETAsio` | 网络组件、当前MockProxy代理及网络实验组件 |
| `服务端/4DyuchiNETStub` | 网络兼容组件 |
| `服务端/Client` | 当前客户端源码，主要工程为`Debug/dragon.vcxproj` |
| `服务端/Library` | 共享源码与头文件 |
| `服务端/tests` | 游戏协议、资源读取和玩法相关C++测试 |
| `服务端/config/examples` | 服务端配置示例及客户端网络配置片段 |
| `source-manifest.json` | 公开文件大小和SHA256清单 |

## 首次准备

```powershell
git clone https://github.com/FungCoder/dragonraja.git D:\DragonRajaSource
Set-Location D:\DragonRajaSource
```

编译命令在源码根目录执行；部署命令在独立运行目录执行。建议源码位于`D:\DragonRajaSource`，运行环境位于`D:\DragonRajaServer`，使用英文路径。

| 材料 | 提供情况 |
|---|---|
| 客户端/服务端源码、工程和共享代码 | 提供 |
| 当前MySQL结构、存储过程和脱敏基础游戏数据 | 提供，见`database` |
| 玩家、角色、邮件、交易和历史日志记录 | 不提供，相关表保留结构 |
| 服务INI模板 | 提供AgentServer、RajaDB及单张MapServer示例，需填写本地参数 |
| 地图、物件、门碰撞、SKB、脚本及完整客户端资源 | 需合法另行准备 |
| EXE/DLL、第三方LIB及SDK | 需自行编译或合法另行准备 |

导入数据库后仍需补齐依赖、完整配置和游戏资源，才能启动服务并进入游戏。

## 从安装到进入游戏

建议先完成同一台电脑上的测试，再配置其他电脑连接。按下面顺序执行，每一步通过后再继续：

1. 准备Windows x64、VS2022构建环境、MySQL 8、x64 ODBC ANSI驱动和合法完整客户端资源。
2. 克隆源码，按[数据库安装说明](database/README.md)校验并导入三个SQL，确认表及存储过程齐全。
3. 准备MySQL连接账号及相应库权限；按数据库说明创建独立的游戏测试账号。MySQL账号用于服务连接，游戏账号用于客户端登录。
4. 编译网络DLL、MockProxy、RajaDB、AgentServer和MapServer；客户端使用匹配版本的程序或按Win32工程编译。
5. 按运行目录表安装程序、资源和配置模板，填写数据库连接字段，配置初始地图并注册网络组件。
6. 启动代理、数据库服务、登录服务及选定地图，完成完整地图目录协商，确认没有启动错误。
7. 在完整客户端目录修改`dragon.ini`的IP和端口，先检查登录端口连通，再从客户端资源目录启动程序。
8. 选择服务器，输入游戏账号，创建角色，选择已经启动的出生地图并进入游戏。
9. 验证移动、NPC、打怪及保存，正常退出后再次登录，确认角色和物品保持一致，再测试跨图。
10. 同机测试通过后，再设置局域网IP或公网/NAT地址，重新检查客户端能访问的地址和服务路由。

只有源码和SQL还不能进入游戏。必须取得匹配客户端、地图及其他资源；本文提供完整操作顺序和可核对的配置入口，不代表已经完成全新机器或公网环境验收。

## 数据库使用说明

使用`database`中的三个文件安装数据库。详细导入步骤、账号准备、校验及预期结果见[数据库安装说明](database/README.md)。

| 内部连接别名 | MySQL库 | 用途 |
|---|---|---|
| DragonDB | `dragonraja` | 角色、物品、技能、NPC、地图及游戏业务数据 |
| TotalDB | `dragonraja_total` | 账号登录、在线路由及跨服务数据 |
| ChrLogDB | `dragonraja_log` | 游戏日志相关数据 |

安装基线保留282张表、103个存储过程，125张基础数据表共38,930行。账号、角色及运维/日志表为空；地图地址泛化为回环地址，历史在线和占用状态已清除。自行创建本地测试账号，并配置实际部署地址。

历史验证环境为MySQL 8.0.12及x64 MySQL ODBC 9.7 ANSI Driver。导出SQL包含MySQL 8排序规则，其他版本、MariaDB及跨操作系统导入未完整验证。

连接映射见`服务端/DBDemon/MySql.cpp`。登录路径见`服务端/DBDemon/Pay.cpp`，调用`dragonraja_total.up_get_user_info2(?)`，按`id_index、passwd、d_kyulje、d_eday、timeremain`顺序读取结果。修改返回列序会破坏登录契约。

## 源码编译说明

### 开发环境

安装Visual Studio 2022或Build Tools 2022的“使用C++的桌面开发”、MSVC v143和Windows SDK。在VS开发者PowerShell或命令提示符中运行`msbuild`。

网络组件需要Boost头文件，历史使用1.83.0；`BOOST_ROOT`指向直接包含`boost`子目录的解压根目录。主要服务端工程使用Release/x64，客户端使用Release/Win32。

| 组件 | 工程 |
|---|---|
| 网络COM组件 | `服务端/4DyuchiNETAsio/4DyuchiNETAsio.vcxproj` |
| 当前代理 | `服务端/4DyuchiNETAsio/MockProxy/MockProxy.vcxproj` |
| 数据库服务 | `服务端/DBDemon/RajaDB.vcxproj` |
| 登录服务 | `服务端/AgentServer/AgentServer.vcxproj` |
| 经典地图服务 | `服务端/Mapserver/MapServer.vcxproj` |
| 客户端 | `服务端/Client/Debug/dragon.vcxproj` |

### 编译服务端

在源码根目录执行，构建失败时先解决实际错误：

```powershell
$env:BOOST_ROOT = 'D:\Dependencies\boost_1_83_0'
$serverOutput = 'D:\DragonRajaBuild\server\'
$projects = @(
    '服务端\4DyuchiNETAsio\4DyuchiNETAsio.vcxproj',
    '服务端\4DyuchiNETAsio\MockProxy\MockProxy.vcxproj',
    '服务端\DBDemon\RajaDB.vcxproj',
    '服务端\AgentServer\AgentServer.vcxproj',
    '服务端\Mapserver\MapServer.vcxproj'
)
foreach ($project in $projects) {
    msbuild $project /m:1 /p:Configuration=Release /p:Platform=x64 "/p:OutDir=$serverOutput"
    if ($LASTEXITCODE -ne 0) { throw "构建失败：$project" }
}
```

单独指定输出目录，避免不同solution默认输出路径混淆。运行组件是经典MapServer和MockProxy；`MapServerAsio`、`MockMapClient`为网络实验/测试组件，不替代完整地图玩法服务。旧`ProxyServer`保留参考代码，有已知SDK声明冲突。

MapServer Release/x64完整构建及部分AgentServer/DBDemon编译单元已验证；尚未完成全部工程的全新机器构建验收。

### 编译客户端

```powershell
msbuild '服务端\Client\Debug\dragon.vcxproj' /m:1 /p:Configuration=Release /p:Platform=Win32 /p:OutDir=D:\DragonRajaBuild\client\
if ($LASTEXITCODE -ne 0) { throw '客户端构建失败' }
```

客户端仍依赖HSEL、Shared、旧JPEG/加密库和DirectX/媒体接口材料，如`ijl11.lib`、`DesDll.lib`等。需取得合法、匹配Win32的依赖，并在工程中设置include/lib路径。`dragonraja.exe`需要匹配的完整客户端资源才能运行。

## 服务端使用说明

### 运行环境与目录

准备Windows x64、Microsoft Visual C++ 2015–2022 x64运行库、MySQL 8及x64 MySQL ODBC ANSI驱动。运行目录示例：

```text
DragonRajaServer/
  exe/AgentServer.exe, MockProxy.exe, 4DyuchiNETAsio.dll, agentserver.ini
  DBDemon/exe/RajaDB.exe, 4DyuchiNETAsio.dll, DBDemon.ini
  runtime/maps/<地图实例>/MapServer.exe, MapServer.ini, 所需DLL及实例依赖
  resources/map/, data/, script/ 等资源
  logs/
```

各服务从自己的工作目录读取INI和依赖。地图放在独立实例目录，配置保持正确地图名称和唯一端口。

### 配置数据库与网络

在源码根目录，将服务配置示例复制到运行目录。先创建对应目录，再执行：

```powershell
$runtimeDirectories = @('D:\DragonRajaServer\exe', 'D:\DragonRajaServer\DBDemon\exe',
    'D:\DragonRajaServer\runtime\maps\ma_in', 'D:\DragonRajaServer\logs')
New-Item -ItemType Directory -Path $runtimeDirectories -Force | Out-Null
Copy-Item '服务端\config\examples\agentserver.ini.example' 'D:\DragonRajaServer\exe\agentserver.ini'
Copy-Item '服务端\config\examples\DBDemon.ini.example' 'D:\DragonRajaServer\DBDemon\exe\DBDemon.ini'
Copy-Item '服务端\config\examples\MapServer.ini.example' 'D:\DragonRajaServer\runtime\maps\ma_in\MapServer.ini'
```

这些是同机配置起点，不是填好账号即可保证运行的安装器。示例中的凭据为空、路径使用`D:\DragonRajaServer`；按实际安装路径修改。`dragon_id/pw`、`total_id/pw`、`chrlog_id/pw`分别配置三个数据库连接；`mysql_conn/server、port、driver`设置MySQL主机、端口及实际驱动名称。也可参考[数据库连接字段](服务端/config/examples/db-account.ini.example)。

用同一游戏数据库账号连接三个库时，在六个账号/口令字段中分别填写对应值；分库使用不同账号时按库填写。驱动名称应与本机安装名称完全一致。数据库默认端口3306，地图和核心服务使用各自端口。

配置中的主要字段如下，以对应源码读取逻辑为准：

| 配置 | 作用 |
|---|---|
| `server_set_info/own_server_set_number` | 同组服务身份，源码要求非零 |
| `server_info/own_server_ip_for_server`和对应端口 | 内部服务连接 |
| `server_info/own_server_ip_for_user`和对应端口 | 用户侧监听 |
| `server_info/primary_proxy_server_ip`和对应端口 | 主代理地址 |
| `server_info/secondary_proxy_server_ip`和对应端口 | 次代理配置 |
| `nation_set/nation_name`、`BETA_SERVER` | 地区与免费/收费模式 |
| 地图资源路径、地图名称及`option/LogPath` | 资源加载和日志 |
| `external_server_info` | 外部服务目录，按完整模板和拓扑配置 |

协议按端口范围区分组件：代理3000～3999、数据库服务4000～4999、地图5000～6999、登录服务7000～7999。历史同机配置使用代理3000、RajaDB 4100、AgentServer内部7001、用户登录7000；地图各用独立端口。MySQL 3306与RajaDB 4100是不同端口。

`map_info.port`、`mapname/mapnameinfo`地址和端口、`mapserverinfo`地址及路径应与地图INI一致。公开基线中的回环地址和`.`路径是同机配置起点；换机或跨机部署需按实际环境设置。客户端目标地址、内部服务IP和用户监听地址分别配置。

在64位PowerShell中注册网络DLL：

```powershell
& "$env:SystemRoot\System32\regsvr32.exe" /s 'D:\DragonRajaServer\exe\4DyuchiNETAsio.dll'
if ($LASTEXITCODE -ne 0) { throw '网络组件注册失败' }
```

组件使用当前Windows用户的COM配置，服务应由同一用户运行；更换服务账号需另行注册核验。

### 地图资源与启动

每张地图需可靠数据库目录、唯一名称和端口、地形、标准TOI2物件和门碰撞、SKB及相应脚本。不能只复制EXE或用别的地图地形补缺。配置模板需按实际地图调整，完整游戏资源另行准备。

MapServer示例对应`MA-IN`、端口5000。其他实例可复制此模板，修改`network/mapname`、`own_server_port_for_server`及`own_server_port_for_user`，端口必须与`dragonraja.map_info`一致。在mysql客户端查询实际目录：

```sql
SELECT number,TRIM(mapfile) AS mapfile,port,nation,forrookie
FROM dragonraja.map_info
WHERE UPPER(TRIM(mapfile)) IN ('MA-IN','K_SUNG2','SCHOLIUM2');
```

首次测试通常需要准备`MA-IN`（5000）、`K_SUNG2`（5010）以及选择新手出生地时的`SCHOLIUM2`（5830）；实际以数据库和角色选择为准。创建对应实例目录，分别安装MapServer.exe和网络DLL，再复制并调整INI，不能让多个实例使用同一个端口。

服务端资源根目录为INI中配置的`resources`，需包含`map/data/script`等所需目录；MAP/TOI2/SKB和脚本必须与地图名匹配。客户端物件可能放在分组目录，不能把整个`object`目录当成标准`map/*.toi2`资源。资源不匹配时先修正对应地图文件，不要仅以进程未退出判断可用。

RajaDB代码会尝试读取资源目录中的`data/IdPassword.bin`，成功时会覆盖INI中的数据库凭据；新环境资源不要携带其他环境的该文件。MapServer示例的`mysql_conn/use_legacy_id_password=0`使用本地INI凭据。

在运行根目录先验证代理参数；示例首张地图端口5000须按实际配置替换：

```powershell
Set-Location D:\DragonRajaServer
.\exe\MockProxy.exe 3000 127.0.0.1 7001 4100 5000 --validate-config
if ($LASTEXITCODE -ne 0) { throw '代理参数检查失败' }
```

参数顺序：代理端口、内部IP、AgentServer内部端口、RajaDB端口、首张地图端口。分别在对应工作目录启动：

| 顺序 | 工作目录 | 命令 |
|---|---|---|
| 1 | `D:\DragonRajaServer\exe` | `MockProxy.exe 3000 127.0.0.1 7001 4100 5000` |
| 2 | `D:\DragonRajaServer\DBDemon\exe` | `RajaDB.exe` |
| 3 | `D:\DragonRajaServer\exe` | `AgentServer.exe` |
| 4 | 每个`runtime/maps/<地图实例>`目录 | `MapServer.exe` |

检查进程、监听端口和对应日志。经典服务在握手时取得地图目录：所有目标地图注册后，保持代理及地图注册存在，重新启动RajaDB和AgentServer，再逐张重启地图，使其协商完整目录。全过程先确认无人在线，完成目录和连接检查后再接入客户端；增加地图也需重新协商完整目录。

先开少量地图，确认内存和日志正常后再增加。103条地图目录记录不代表资源齐全或均可开服；此前准备过98张有地形目录地图，只完成部分地图互联验证。

### 停服与验收

先让玩家正常退出并确认角色保存，再关闭地图、登录服务、数据库服务和代理。关闭MySQL不能代替停止游戏服务，有玩家在线时不能直接强制结束进程。

首次验收包括组件监听与互联、登录、创角、进图、保存及再次登录、跨图、NPC交互、经验和掉落、商店、邮件/仓库及停服重启。端口监听或数据库导入成功不能替代游戏内验收。

公网/NAT部署尚未完整验证。需实际核验客户端地址、内外IP、登录和地图端口映射，不保证只开放登录端口即可游玩。

## 客户端配置与进入游戏

### 1. 准备完整客户端

将合法取得、与服务端匹配的完整客户端放在英文目录，例如`D:\DragonRajaClient`。目录内应有`dragonraja.exe`、原有`dragon.ini`、所需32位DLL、语言数据以及`data/map/object`等原有资源。若自己编译客户端，把生成的EXE复制到这份完整客户端中，不能只复制EXE到空目录。

客户端使用相对路径读取部分资源。双击EXE前先确认文件位于完整资源目录；通过快捷方式启动时，“起始位置”设为该目录。

### 2. 修改客户端IP和端口

关闭客户端，用文本编辑器打开与`dragonraja.exe`同目录的`dragon.ini`，找到已有`[network]`节，在其中修改以下字段。不要另建同名节，也不要覆盖其余设置或改变原文件编码。

下面是**同机测试网络片段**，可参考[客户端网络示例](服务端/config/examples/dragon-network.ini.example)。合并到原INI中，保留原有`ver`和其他设置：

```ini
[network]
host1=127.0.0.1
host2=127.0.0.1
server_count=1
name1=Local
name1_host1=127.0.0.1
name1_host2=127.0.0.1
ServerSet=0
proxy_port=3000
agent_port=7000
direct_agent=1
```

| 字段 | 设置方法 |
|---|---|
| `host1`、`host2` | 均填写客户端能访问的服务器IPv4；同机使用127.0.0.1 |
| `name1_host1`、`name1_host2` | 第一项服务器的地址，与上面的IP一致 |
| `name1` | 服务器列表显示名称，示例使用ASCII名称Local |
| `server_count` | 示例只有一项服务器，设为1 |
| `ServerSet` | 默认选择的列表索引；第一项为0 |
| `agent_port` | AgentServer用户侧登录端口，示例7000；不是内部7001 |
| `proxy_port` | 代理查询端口，示例3000；不是MySQL端口3306 |
| `direct_agent` | 当前客户端设为1直接连接登录服务；设为0则先从代理获取登录地址 |
| `ver` | 保留与程序/服务端匹配的原值，必须能读取为非零，不能删除或自行猜测 |

当前连接代码使用`inet_addr`，填写点分IPv4，不要填`https://`、域名或IPv6。修改以上字段无需重新编译客户端；没有这些读取逻辑的其他版本EXE可能不支持`direct_agent`，需使用与当前工程匹配的程序。

有多项服务器时，分别检查`name2_host1/name2_host2`等字段。服务器选择界面会读取对应列表地址，不能只改`host1`；直接连接模式会在启动时读取`host1`，因此当前单服务器配置将四个地址保持一致。

### 3. 同机、局域网与公网地址

| 情况 | 客户端四个host字段 | 服务端对应设置 |
|---|---|---|
| 客户端和服务端在同一电脑 | 127.0.0.1 | 同机示例，登录监听7000 |
| 同一局域网的另一台电脑 | 服务器的局域网IPv4，例如192.168.1.50 | AgentServer用户监听使用该网卡地址或0.0.0.0；客户端可见地址使用实际局域网IP |
| 公网/NAT连接 | 服务器公网IPv4 | 配置外部端口映射、用户监听和客户端可见地址，目标网络须实测 |

`0.0.0.0`用于服务端监听所有网卡，不能作为客户端目标地址。另一台电脑上的127.0.0.1指那台电脑自己。所有核心服务同机运行时，内部代理/RajaDB/AgentServer连接仍可用回环地址；不要把内部IP全部机械替换为公网IP。

AgentServer的`server_info/own_server_ip_for_user`是绑定监听地址，`advertised_player_ip`是对外发送的可见地址；跨机时按实际网络配置。地图目录和路由也需一致，登录端口连通不代表进图与传送已经通过。

### 4. 检查连接并启动客户端

在客户端电脑的PowerShell中先检查7000登录端口；局域网或公网测试将地址换成对应服务器IPv4：

```powershell
Test-NetConnection -ComputerName 127.0.0.1 -Port 7000
```

`TcpTestSucceeded=True`只表示端口可达。若为False，先检查AgentServer进程、实际监听IP/端口及对应防火墙/端口映射，不要反复改账号密码。

在完整客户端目录启动窗口模式：

```powershell
Set-Location D:\DragonRajaClient
.\dragonraja.exe -w -r800x600
```

也可使用`-w -r1024x768`。不要用其他环境的启动器覆盖你刚修改的INI，发现地址恢复旧值时检查启动器行为和是否改错了客户端目录。

### 5. 登录、创角和进入游戏

1. 在服务器列表选择你设置的名称，进入登录界面；使用[数据库说明](database/README.md#4-配置数据库用户和本地测试账号)准备的**游戏账号**及口令，不是MySQL管理员账号。
2. 首次登录没有角色是正常的，点击新建角色。按该客户端界面选择国家、性别、职业、外观等必填项，输入符合界面限制且未被占用的角色名，完成创建。界面按钮文字可能随语言资源不同。
3. 选中角色，选择已经启动并已加载完整资源的出生地。若选择新手地区，服务端必须有对应的SCHOLIUM/SCHOLIUM2实例；只开MA-IN不保证所有出生选项可进入。角色上次所在地图、城镇和传送目标也要实际在线。
4. 等待地图加载，确认角色可见、可移动并可与NPC交互。登录成功但卡在加载时，检查出生地图、完整服务目录协商和服务端错误日志；不应改口令来处理地图故障。
5. 测试打怪、经验与掉落、商店和背包，使用游戏正常退出功能离线，再重新登录确认角色和物品已保存。跨图前先启动目标地图并重新协商完整目录。

客户端的“注册”按钮不能替代本站数据库账号创建，也没有随源码提供注册网站。免费测试使用RajaDB/AgentServer配置中的`NATION_SET/BETA_SERVER=1`，仍会验证账号口令；收费模式按账号到期时间与计费规则处理。

### 6. 客户端常见问题

| 现象 | 检查内容 |
|---|---|
| 修改IP后仍连旧地址 | 是否编辑EXE同目录dragon.ini、四个host字段是否一致、启动器是否改写文件 |
| 启动即退出或Init失败 | 原有ver是否缺失/为0、host1/host2是否为空、资源及运行依赖是否齐全 |
| 报语言包或地图文件错误 | 是否从完整客户端目录启动，语言/地图资源与程序版本是否匹配 |
| 无法连接或连接超时 | IPv4、agent_port、direct_agent及AgentServer用户监听；不要误用7001或3306 |
| 账号不存在或密码错误 | 总库账号、口令与登录过程，游戏账号和数据库账号不能混用 |
| 登录后提示计费/到期 | BETA_SERVER、有效期和计费配置是否一致 |
| 创角失败或进图加载不结束 | 角色名规则、数据库错误、出生地实例和资源、完整地图目录协商 |
| 退出再登录进不了原地图 | 上次角色地图是否在线及路由是否正确 |
| 窗口/图像异常 | 先使用800x600窗口模式，核对显示环境及匹配的客户端资源 |

## 常见故障

| 现象 | 检查内容 |
|---|---|
| 缺Boost头文件 | BOOST_ROOT是否直接包含boost目录 |
| 客户端缺LIB或位数冲突 | 旧依赖、Win32位数及工程库路径 |
| ODBC驱动不存在 | 实际驱动名称、ANSI/x64、INI位置和工作目录 |
| 数据库拒绝连接 | 数据库用户、来源主机、密码、端口和权限 |
| 游戏登录失败 | 总库账号、过程列序、免费/收费配置及连接超时 |
| 启动后退出/端口占用 | 重复实例、日志、COM注册及DLL依赖 |
| 地图缺资源 | 地图名称、路径、地形/物件/SKB和脚本 |
| 互联成功但无法进图/传送 | 客户端资源、路由、内外IP和地图目录协商 |

## 当前验证范围

三个SQL已在独立MySQL 8.0.12实例重导入，282张表的数量和逐表行数核对通过，总库登录过程对不存在账号返回空结果。

尚未完成全新机器开服、客户端旧依赖补齐或全地图玩法验收。历史缺地形地图为KAELUNE、FIGHT2、HOUSE、HILL、ITEMSEARCH；SN_2F、SOCCER缺可靠目录元数据。首领阶段、国战、静态门/箱和部分经验模板仍待核验，不能称为完整原版恢复。

## 反馈与交流

使用过程中遇到问题，或有建议需要交流，欢迎到 [Issues](https://github.com/FungCoder/dragonraja/issues) 反馈。请尽量提供运行环境、复现步骤和相关错误信息，方便排查。
