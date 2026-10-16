#include "stdafx.h"
#include "AutoPlay.h"
#include "CombatShotClient.h"
#include "AutoPlayActions.h"
#include "AutoPlayExtras.h"
#include "AutoPlayModel.h"
#include "Dragon.h"
#include "Char.h"
#include "CharDataTable.h"
#include "Menu.h"
#include "GameProc.h"
#include "Map.h"
#include "Path.h"
#include "BattleMgr.h"
#include "Effect.h"
#include "Skill.h"
#include "skill_lsw.h"
#include <algorithm>
#include <string>
#include <vector>

extern void DoHeroMove(int moveType, int x, int y);
extern void DoHeroAttack(int x, int y);

namespace
{
using namespace DragonAutoPlayClient;
const int RowCount = 11;
const int RecoveryRow = 11;
const int ProbeRow = -1;
struct Row
{
    bool enabled = false, self = false, monster = true, expired = false, continuous = false;
    int kind = DragonAutoPlay::Attack, skill = 0, seconds = 0;
};
struct Configuration
{
    Row rows[RowCount];
    int thresholds[3] = {0, 0, 0};
    int quickSlots[3] = {-1, -1, -1};
    bool work = false, autoCp = false, switchWeapon = false;
    bool hideMagic = false, hidePlayers = false, autoKick = false, range = false, renew = true;
    int radius = 6, x = 0, y = 0;
    Configuration()
    {
        rows[0].enabled = true;
        for (int i = 1; i < RowCount; ++i) rows[i].skill = 65535;
        rows[10].kind = DragonAutoPlay::Work;
        rows[10].monster = false;
    }
};
Configuration config;
struct Controls { HWND enabled, seconds, skill, self, monster, expired, continuous; } controls[RowCount];
HWND windowHandle = NULL, statusHandle = NULL;
HWND modeWork, modeBattle, thresholdControls[3], quickControls[3];
HWND cpControl, weaponControl, magicControl, playersControl, kickControl, rangeControl, renewControl;
HWND radiusControl, xControl, yControl;
HFONT uiFont = NULL;
std::vector<ActionOption> options;
bool loaded = false, running = false, negotiating = false, preparing = false, executing = false;
bool replyReady = false, magicReady = false;
int activeRow = ProbeRow, nextRow = 0, mapAtStart = -1, actorAtStart = 0;
int centerX = 0, centerY = 0, pendingSkill = 0, pendingTarget = 0;
uint32_t due[RowCount] = {}, nextQuery = 0, actionDeadline = 0, itemDeadline = 0, weaponDeadline = 0;
uint32_t cpDeadline = 0;
bool alternateTarget[RowCount] = {};
uint16_t sequence = 0;
QueryTicket ticket;
DragonAutoPlay::Response reply = {};
std::wstring statusText = L"尚未开始";

void Status(const wchar_t* text)
{
    statusText = text;
    if (statusHandle) SetWindowTextW(statusHandle, text);
}
bool Checked(HWND control) { return SendMessageW(control, BM_GETCHECK, 0, 0) == BST_CHECKED; }
void Check(HWND control, bool value) { SendMessageW(control, BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0); }
HWND Control(const wchar_t* type, const wchar_t* title, DWORD style, int x, int y, int width, int height, int id)
{
    HWND handle = CreateWindowExW(0, type, title, WS_CHILD | WS_VISIBLE | style,
        x, y, width, height, windowHandle, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), g_hInstance, NULL);
    SendMessageW(handle, WM_SETFONT, reinterpret_cast<WPARAM>(uiFont), TRUE);
    return handle;
}
void Number(HWND handle, int value)
{
    wchar_t text[24]; swprintf_s(text, L"%d", value); SetWindowTextW(handle, text);
}
bool ReadNumber(HWND handle, int minimum, int maximum, int& value)
{
    wchar_t text[32]; GetWindowTextW(handle, text, 32);
    wchar_t* end = NULL;
    const long number = wcstol(text, &end, 10);
    if (!*text || !end || *end || number < minimum || number > maximum) return false;
    value = static_cast<int>(number); return true;
}
std::wstring ConfigPath()
{
    wchar_t directory[MAX_PATH];
    if (!GetCurrentDirectoryW(MAX_PATH, directory)) return L"";
    return std::wstring(directory) + L"\\autoplay.ini";
}
int Ini(const std::wstring& file, const wchar_t* section, const wchar_t* key, int fallback)
{
    return static_cast<int>(GetPrivateProfileIntW(section, key, fallback, file.c_str()));
}
void Load()
{
    if (loaded) return;
    loaded = true;
    const std::wstring file = ConfigPath();
    if (Ini(file, L"AutoPlay", L"Version", 0) != 1) return;
    config.work = Ini(file,L"AutoPlay",L"Work",0) != 0;
    config.radius = (std::max)(1,(std::min)(20,Ini(file,L"AutoPlay",L"Radius",6)));
    config.x = Ini(file,L"AutoPlay",L"X",0); config.y = Ini(file,L"AutoPlay",L"Y",0);
    config.autoCp = Ini(file,L"AutoPlay",L"AutoCP",0) != 0;
    config.switchWeapon = Ini(file,L"AutoPlay",L"SwitchWeapon",0) != 0;
    config.hideMagic = Ini(file,L"AutoPlay",L"HideMagic",0) != 0;
    config.hidePlayers = Ini(file,L"AutoPlay",L"HidePlayers",0) != 0;
    config.range = Ini(file,L"AutoPlay",L"Range",0) != 0;
    config.renew = Ini(file,L"AutoPlay",L"Renew",1) != 0;
    for (int resource = 0; resource < 3; ++resource)
    {
        wchar_t section[24]; swprintf_s(section,L"Restore%d",resource);
        config.thresholds[resource] = (std::max)(0,(std::min)(100,Ini(file,section,L"Percent",0)));
        config.quickSlots[resource] = (std::max)(-1,(std::min)(5,Ini(file,section,L"QuickSlot",-1)));
    }
    for (int index = 0; index < RowCount; ++index)
    {
        wchar_t section[24]; swprintf_s(section,L"Row%d",index);
        Row& row = config.rows[index];
        row.enabled = Ini(file,section,L"Enabled",index == 0) != 0;
        row.kind = Ini(file,section,L"Kind",index == 10 ? DragonAutoPlay::Work : DragonAutoPlay::Attack);
        row.skill = Ini(file,section,L"Skill",index == 0 ? 0 : 65535);
        row.seconds = Ini(file,section,L"Seconds",0);
        row.self = Ini(file,section,L"Self",0) != 0;
        row.monster = Ini(file,section,L"Target",index != 10) != 0;
        row.expired = Ini(file,section,L"Expired",0) != 0;
        row.continuous = Ini(file,section,L"Continuous",0) != 0;
        if (!IsIntervalValid(row.seconds) || !IsLearned(row.kind,row.skill)) row.enabled = false;
    }
}
bool Save()
{
    const std::wstring file = ConfigPath();
    wchar_t directory[MAX_PATH], temporary[MAX_PATH];
    if (file.empty() || !GetCurrentDirectoryW(MAX_PATH,directory) || !GetTempFileNameW(directory,L"ap",0,temporary)) return false;
    bool ok = true;
    const auto write = [&](const wchar_t* section,const wchar_t* key,int value) {
        wchar_t text[32]; swprintf_s(text,L"%d",value);
        if (!WritePrivateProfileStringW(section,key,text,temporary)) ok = false;
    };
    write(L"AutoPlay",L"Version",1); write(L"AutoPlay",L"Work",config.work);
    write(L"AutoPlay",L"Radius",config.radius); write(L"AutoPlay",L"X",config.x); write(L"AutoPlay",L"Y",config.y);
    write(L"AutoPlay",L"AutoCP",config.autoCp); write(L"AutoPlay",L"SwitchWeapon",config.switchWeapon);
    write(L"AutoPlay",L"HideMagic",config.hideMagic); write(L"AutoPlay",L"HidePlayers",config.hidePlayers);
    write(L"AutoPlay",L"Range",config.range); write(L"AutoPlay",L"Renew",config.renew);
    for (int resource = 0; resource < 3; ++resource)
    {
        wchar_t section[24]; swprintf_s(section,L"Restore%d",resource);
        write(section,L"Percent",config.thresholds[resource]); write(section,L"QuickSlot",config.quickSlots[resource]);
    }
    for (int index = 0; index < RowCount; ++index)
    {
        wchar_t section[24]; swprintf_s(section,L"Row%d",index);
        const Row& row = config.rows[index];
        write(section,L"Enabled",row.enabled); write(section,L"Kind",row.kind); write(section,L"Skill",row.skill);
        write(section,L"Seconds",row.seconds); write(section,L"Self",row.self); write(section,L"Target",row.monster);
        write(section,L"Expired",row.expired); write(section,L"Continuous",row.continuous);
    }
    if (!WritePrivateProfileStringW(NULL,NULL,NULL,temporary)) ok = false;
    if (ok) ok = MoveFileExW(temporary,file.c_str(),MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != FALSE;
    if (!ok) DeleteFileW(temporary);
    return ok;
}
bool Validate()
{
    int continuous = 0, enabled = 0;
    const int first = config.work ? 10 : 0, last = config.work ? 11 : 10;
    for (int index = first; index < last; ++index)
    {
        const Row& row = config.rows[index];
        if (!row.enabled) continue;
        if (!IsIntervalValid(row.seconds) || !IsLearned(row.kind,row.skill) || (!row.self && !row.monster))
        { Status(L"请检查已启用行的技能、间隔和释放目标"); return false; }
        if (index == 10 && !IsWorkSupported(row.skill))
        { Status(L"当前工作技能不支持自动重复，请选择采集技能"); return false; }
        if (row.expired && row.kind == DragonAutoPlay::Attack)
        { Status(L"普通攻击不能设置为增益到期补充"); return false; }
        continuous += row.continuous ? 1 : 0; ++enabled;
    }
    if (!enabled || continuous > 1) { Status(L"至少启用一条配置，持续释放最多选择一条"); return false; }
    if (config.work && (config.x < 0 || config.y < 0 || config.x >= g_Map.file.wWidth || config.y >= g_Map.file.wHeight))
    { Status(L"工作坐标超出当前地图"); return false; }
    return true;
}
bool ReadControls()
{
    if (!windowHandle) return Validate();
    for (int index = 0; index < RowCount; ++index)
    {
        Row& row = config.rows[index]; const Controls& ui = controls[index];
        row.enabled = Checked(ui.enabled); row.self = Checked(ui.self); row.monster = Checked(ui.monster);
        row.expired = Checked(ui.expired); row.continuous = Checked(ui.continuous);
        if (!ReadNumber(ui.seconds,0,86400,row.seconds)) { Status(L"释放间隔应为0到86400秒"); return false; }
        const LRESULT selection = SendMessageW(ui.skill,CB_GETCURSEL,0,0);
        const LRESULT option = selection == CB_ERR ? -1 : SendMessageW(ui.skill,CB_GETITEMDATA,selection,0);
        if (option < 0 || static_cast<size_t>(option) >= options.size()) { row.skill = 65535; }
        else { row.kind = options[option].kind; row.skill = options[option].skill; }
    }
    for (int resource = 0; resource < 3; ++resource)
    {
        if (!ReadNumber(thresholdControls[resource],0,100,config.thresholds[resource]))
        { Status(L"恢复阈值应为0到100之间的百分比"); return false; }
        config.quickSlots[resource] = static_cast<int>(SendMessageW(quickControls[resource],CB_GETCURSEL,0,0)) - 1;
    }
    if (!ReadNumber(radiusControl,1,20,config.radius) || !ReadNumber(xControl,0,32767,config.x) || !ReadNumber(yControl,0,32767,config.y))
    { Status(L"请检查范围与工作坐标"); return false; }
    config.work = Checked(modeWork);
    config.autoCp = Checked(cpControl); config.switchWeapon = Checked(weaponControl);
    config.hideMagic = Checked(magicControl); config.hidePlayers = Checked(playersControl);
    config.autoKick = SupportsAutoKick() && Checked(kickControl);
    config.range = Checked(rangeControl); config.renew = Checked(renewControl);
    return Validate();
}
void Populate()
{
    options = GetActionOptions();
    for (int index = 0; index < RowCount; ++index)
    {
        const Controls& ui = controls[index]; const Row& row = config.rows[index];
        Check(ui.enabled,row.enabled); Check(ui.self,row.self); Check(ui.monster,row.monster);
        Check(ui.expired,row.expired); Check(ui.continuous,row.continuous); Number(ui.seconds,row.seconds);
        SendMessageW(ui.skill,CB_RESETCONTENT,0,0);
        SendMessageW(ui.skill,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"无"));
        SendMessageW(ui.skill,CB_SETITEMDATA,0,-1);
        int selected = 0;
        for (size_t optionIndex = 0; optionIndex < options.size(); ++optionIndex)
        {
            const ActionOption& option = options[optionIndex];
            if ((index == 10) != (option.kind == DragonAutoPlay::Work)) continue;
            const wchar_t* category = (option.classification & WorkAction) ? L"[工作] " :
                (option.classification & CombatRecovery) ? L"[战技恢复] " :
                (option.classification & RestorationAction) ? L"[治疗恢复] " :
                (option.classification & BuffAction) ? L"[增益] " :
                (option.classification & DebuffAction) ? L"[诅咒] " :
                (option.classification & DragonAutoPlayClient::AttackAction) ? L"[攻击] " : L"[辅助] ";
            const std::wstring label = category + option.label;
            const LRESULT item = SendMessageW(ui.skill,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(label.c_str()));
            SendMessageW(ui.skill,CB_SETITEMDATA,item,optionIndex);
            if (row.kind == option.kind && row.skill == option.skill) selected = static_cast<int>(item);
        }
        SendMessageW(ui.skill,CB_SETCURSEL,selected,0);
    }
    for (int resource = 0; resource < 3; ++resource)
    {
        Number(thresholdControls[resource],config.thresholds[resource]);
        SendMessageW(quickControls[resource],CB_SETCURSEL,config.quickSlots[resource]+1,0);
    }
    Check(modeBattle,!config.work); Check(modeWork,config.work);
    Check(cpControl,config.autoCp); Check(weaponControl,config.switchWeapon);
    Check(magicControl,config.hideMagic); Check(playersControl,config.hidePlayers);
    Check(rangeControl,config.range); Check(renewControl,config.renew);
    Number(radiusControl,config.radius); Number(xControl,config.x); Number(yControl,config.y);
    SetWindowTextW(statusHandle,statusText.c_str());
}
LRESULT CALLBACK SettingsProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CLOSE) { ShowWindow(handle,SW_HIDE); return 0; }
    if (message == WM_COMMAND)
    {
        switch (LOWORD(wParam))
        {
        case 1:
            if (ReadControls())
            {
                if (!Save()) Status(L"设置保存失败，请检查目录权限");
                else { ShowWindow(handle,SW_HIDE); SetForegroundWindow(g_hwndMain); AutoPlayStart(); }
            }
            return 0;
        case 2: AutoPlayStop(); ShowWindow(handle,SW_HIDE); SetForegroundWindow(g_hwndMain); return 0;
        case 3: if (ReadControls()) Status(Save() ? L"设置已保存" : L"设置保存失败，请检查目录权限"); return 0;
        }
    }
    return DefWindowProcW(handle,message,wParam,lParam);
}
void CreateSettings()
{
    WNDCLASSW windowClass = {};
    windowClass.lpfnWndProc = SettingsProc; windowClass.hInstance = g_hInstance;
    windowClass.hCursor = LoadCursor(NULL,IDC_ARROW); windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE+1);
    windowClass.lpszClassName = L"DragonAutoPlaySettings";
    RegisterClassW(&windowClass);
    windowHandle = CreateWindowExW(WS_EX_TOOLWINDOW,L"DragonAutoPlaySettings",L"内置挂机",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT,CW_USEDEFAULT,930,700,g_hwndMain,NULL,g_hInstance,NULL);
    if (!windowHandle) return;
    uiFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    const wchar_t* resources[3] = {L"血量低于",L"魔力低于",L"饱食度低于"};
    for (int resource = 0; resource < 3; ++resource)
    {
        const int y = 12+resource*28;
        Control(L"STATIC",resources[resource],0,15,y+4,80,22,0);
        thresholdControls[resource] = Control(L"EDIT",L"0",WS_BORDER | ES_NUMBER,100,y,45,22,0);
        Control(L"STATIC",L"% 时使用快捷栏恢复物品（间隔10秒）",0,150,y+4,290,22,0);
        quickControls[resource] = Control(L"COMBOBOX",L"",CBS_DROPDOWNLIST | WS_VSCROLL,450,y,220,180,0);
        SendMessageW(quickControls[resource],CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"自动选择对应恢复物品"));
        for (int slot=0;slot<6;++slot) { wchar_t text[32];swprintf_s(text,L"快捷栏 %d",slot+1);SendMessageW(quickControls[resource],CB_ADDSTRING,0,reinterpret_cast<LPARAM>(text)); }
    }
    modeBattle = Control(L"BUTTON",L"战斗挂机",BS_AUTORADIOBUTTON | WS_GROUP,15,102,120,24,10);
    Control(L"STATIC",L"启用   间隔(秒)       技能/魔法                     释放目标           到期补充       持续释放",0,15,132,850,22,0);
    for (int index=0;index<RowCount;++index)
    {
        const int y = index < 10 ? 156+index*28 : 566;
        Controls& ui=controls[index];
        ui.enabled=Control(L"BUTTON",L"",BS_AUTOCHECKBOX,15,y,24,22,100+index);
        ui.seconds=Control(L"EDIT",L"0",WS_BORDER | ES_NUMBER,52,y,70,22,200+index);
        ui.skill=Control(L"COMBOBOX",L"",CBS_DROPDOWNLIST | WS_VSCROLL,135,y,265,240,300+index);
        ui.self=Control(L"BUTTON",L"自己",BS_AUTOCHECKBOX,414,y,65,22,400+index);
        ui.monster=Control(L"BUTTON",index==10?L"坐标":L"怪物",BS_AUTOCHECKBOX,485,y,65,22,500+index);
        ui.expired=Control(L"BUTTON",L"到期补充",BS_AUTOCHECKBOX,574,y,95,22,600+index);
        ui.continuous=Control(L"BUTTON",L"持续释放",BS_AUTOCHECKBOX,690,y,100,22,700+index);
        if(index==10) EnableWindow(ui.expired,FALSE);
    }
    cpControl=Control(L"BUTTON",L"CP小于25自动恢复",BS_AUTOCHECKBOX,15,443,195,22,20);
    weaponControl=Control(L"BUTTON",L"自动切换武器",BS_AUTOCHECKBOX,225,443,160,22,21);
    magicControl=Control(L"BUTTON",L"隐藏魔法效果",BS_AUTOCHECKBOX,405,443,160,22,22);
    playersControl=Control(L"BUTTON",L"隐藏其他玩家",BS_AUTOCHECKBOX,585,443,170,22,23);
    rangeControl=Control(L"BUTTON",L"范围移动挂机",BS_AUTOCHECKBOX,15,472,170,22,24);
    Control(L"STATIC",L"范围(格)",0,190,476,75,22,0);
    radiusControl=Control(L"EDIT",L"6",WS_BORDER | ES_NUMBER,270,472,45,22,0);
    renewControl=Control(L"BUTTON",L"增益到期自动补充",BS_AUTOCHECKBOX,335,472,210,22,25);
    kickControl=Control(L"BUTTON",L"自动移除离开3分钟队友",BS_AUTOCHECKBOX,560,472,250,22,26);
    EnableWindow(kickControl,SupportsAutoKick());
    Control(L"STATIC",L"队伍离线状态尚未同步时，该选项不可用。0秒配置仍遵守服务端技能冷却。",0,15,503,845,22,0);
    modeWork=Control(L"BUTTON",L"工作挂机",BS_AUTORADIOBUTTON,15,534,130,22,11);
    Control(L"STATIC",L"工作坐标",0,15,601,80,22,0);
    xControl=Control(L"EDIT",L"0",WS_BORDER | ES_NUMBER,100,598,60,22,0);
    yControl=Control(L"EDIT",L"0",WS_BORDER | ES_NUMBER,175,598,60,22,0);
    Control(L"BUTTON",L"保存设置",BS_PUSHBUTTON,405,598,115,26,3);
    Control(L"BUTTON",L"开始挂机 L",BS_PUSHBUTTON,535,598,130,26,1);
    Control(L"BUTTON",L"停止 / 取消 P",BS_PUSHBUTTON,680,598,145,26,2);
    statusHandle=Control(L"STATIC",L"",0,15,633,855,23,0);
}
bool Enemy(CHARACTER* character)
{
    return character && character != Hero && character->id >= 10000 &&
        (character->type == SPRITETYPE_MONSTER || character->type == SPRITETYPE_NPC) &&
        !character->eventnpc && character->hp > 0 && !IsDead(character) &&
        character->HostId == 0 && character->tame == 0 && character->sprno >= 2 &&
        character->sprno != 50 && character->nAttr != AT_SUMMON && character->nAttr != AT_TAMING;
}
bool InConfiguredArea(const CHARACTER* character)
{
    const int64_t dx=character->x-centerX,dy=character->y-centerY;
    const int64_t radius=static_cast<int64_t>(config.radius)*TILE_SIZE;
    return dx*dx+dy*dy<=radius*radius;
}
CHARACTER* FindEnemy()
{
    CHARACTER* best=NULL;
    int64_t bestDistance=INT64_MAX;
    const int64_t radius=static_cast<int64_t>(config.radius)*TILE_SIZE;
    for(CHARACTER* current=Hero?Hero->lpNext:NULL;current;current=current->lpNext)
    {
        if(!Enemy(current)) continue;
        const int64_t cx=current->x-centerX, cy=current->y-centerY;
        if(cx*cx+cy*cy>radius*radius) continue;
        const int64_t dx=current->x-Hero->x,dy=current->y-Hero->y;
        const int64_t distance=dx*dx+dy*dy;
        if(distance<bestDistance){best=current;bestDistance=distance;}
    }
    return best;
}
CHARACTER* RowTarget(const Row& row,int index)
{
    if(row.kind==DragonAutoPlay::Work) return Hero;
    if(row.self && (!row.monster || alternateTarget[index])) return Hero;
    CHARACTER* enemy=FindEnemy();
    return enemy ? enemy : (row.self ? Hero : NULL);
}
bool InPhysicalRange(CHARACTER* target)
{
    int minimum=AttackRangeTable[Hero->sprno][Hero->accessory[2]][0];
    int maximum=AttackRangeTable[Hero->sprno][Hero->accessory[2]][1];
    return IsAttackRange(&minimum,&maximum,Hero,target->x,target->y)==ATTACKRANGE_IN_AREA_;
}
void MarkAction(int index,uint32_t now)
{
    if(index>=0 && index<RowCount)
    {
        due[index]=now+ConfiguredIntervalMs(config.rows[index].seconds);
        alternateTarget[index]=!alternateTarget[index];
    }
    if(index==RecoveryRow) cpDeadline=now+1000;
    activeRow=ProbeRow; nextQuery=now+500;
}
void Query(int index,const Row& row,CHARACTER* target,uint32_t now)
{
    DragonAutoPlay::Request request={};
    request.version=DragonAutoPlay::Version; request.kind=static_cast<uint8_t>(row.kind);
    request.skill=static_cast<uint16_t>(row.skill); request.target=static_cast<uint16_t>(target->id);
    request.sequence=++sequence;
    t_packet packet={}; packet.h.header.type=DragonAutoPlay::RequestCommand;
    packet.h.header.size=sizeof(request); memcpy(packet.u.data,&request,sizeof(request));
    QueuePacket(&packet,1); ticket.Begin(request,now); activeRow=index;
}
void ExecuteReply(uint32_t now)
{
    using namespace DragonAutoPlay;
    replyReady=false;
    if(negotiating)
    {
        negotiating=false;
        if(reply.reason==Dead || reply.reason==War || reply.reason==Disabled)
        {AutoPlayStop();Status(L"当前状态或地图不允许挂机");return;}
        Status(L"挂机已开启，P停止；手动点击会中断");nextQuery=now+250;return;
    }
    Row row;
    if(activeRow==RecoveryRow){row.kind=Combat;row.skill=reply.skill;row.self=true;row.monster=false;row.expired=true;}
    else if(activeRow>=0 && activeRow<RowCount) row=config.rows[activeRow];
    else return;
    CHARACTER* target=ReturnCharacterPoint(reply.target);
    if(!target || (target!=Hero && (!Enemy(target)||!InConfiguredArea(target)))) {MarkAction(activeRow,now);return;}
    if(reply.reason==Dead || reply.reason==War || reply.reason==Disabled)
    {AutoPlayStop();Status(L"当前状态或地图不允许挂机");return;}
    if(row.expired)
    {
        if(!(reply.known&EffectKnown))
        {const int index=activeRow;Status(L"此技能暂缺增益状态同步，请使用间隔释放");MarkAction(index,now);if(index>=0&&index<RowCount)due[index]=now+5000;return;}
        if(reply.known&EffectActive)
        {const int index=activeRow;MarkAction(index,now);if(index>=0&&index<RowCount)due[index]=now+(row.self&&row.monster?1000:(std::max)(500u,(std::min)(5000u,reply.effect_remaining_ms)));return;}
    }
    if(!reply.allowed)
    {
        const int index=activeRow;
        if(reply.reason==OutOfRange && config.range && target!=Hero)
        {DoHeroMove(0,target->x,target->y);MarkAction(index,now);return;}
        if(reply.reason==Cooldown && (reply.known&WaitKnown)) nextQuery=now+(std::max)(250u,reply.wait_ms);
        else nextQuery=now+500;
        if(index>=0&&index<RowCount) due[index]=nextQuery;
        if(reply.reason==NotLearned) Status(L"配置中存在未学会或已失效的技能");
        return;
    }
    if(row.kind==Attack)
    {
        if (g_mgrBattle.IsThrow() && CombatShotNeedsHandshake(Hero->nActiveCombat, Hero->bow_Type) && !CombatShotReady())
        { AutoPlayStop(); Status(L"远程攻击暂不可用，请使用配套版本的客户端和服务端"); return; }
        if(!InPhysicalRange(target))
        {
            if(config.range) DoHeroMove(0,target->x,target->y);
            MarkAction(activeRow,now);return;
        }
        CHARACTER* saved=g_GameInfo.lpcharacter;const int savedType=g_GameInfo.nSelectedSpriteType;
        g_GameInfo.lpcharacter=target;g_GameInfo.nSelectedSpriteType=target->type;
        DoHeroAttack(target->x,target->y);
        g_GameInfo.lpcharacter=saved;g_GameInfo.nSelectedSpriteType=savedType;
        MarkAction(activeRow,now);return;
    }
    if((reply.known&RangeKnown) && target!=Hero)
    {
        const int64_t dx=target->x-Hero->x,dy=target->y-Hero->y;
        if(dx*dx+dy*dy>static_cast<int64_t>(reply.range_pixels)*reply.range_pixels)
        {if(config.range)DoHeroMove(0,target->x,target->y);MarkAction(activeRow,now);return;}
    }
    if(row.kind==Magic)
    {
        if(RemainingMs(now,Hero->end_Casting)||RemainingMs(now,Hero->castStartingTime)||Hero->moveFlag)
        {nextQuery=now+500;return;}
        pendingSkill=row.skill;pendingTarget=target->id;magicReady=false;preparing=true;
        actionDeadline=now+70000;
        g_mgrBattle.SendMagicSelect(static_cast<BYTE>(row.skill));
        return;
    }
    if(row.kind==Combat)
    {
        if(RemainingMs(now,Hero->end_Casting)||RemainingMs(now,Hero->castStartingTime)) {nextQuery=now+500;return;}
        if(row.skill<0||row.skill>=MAX_MAGIC){MarkAction(activeRow,now);return;}
        const int category=g_lpMT[row.skill].spell_Type;
        if(category==PASSIVE_COMBAT || category==RECOVERY_COMBAT) g_mgrBattle.SendCombatSelect(static_cast<BYTE>(row.skill));
        else g_mgrBattle.SendCombatAttack(static_cast<BYTE>(row.skill),target,target->x,target->y);
        MarkAction(activeRow,now);nextQuery=now+1000;return;
    }
    if(row.kind==Work)
    {
        TryWork(row.skill,config.x,config.y,row.self);
        MarkAction(activeRow,now);nextQuery=now+2000;
    }
}
}

void AutoPlayShowSettings()
{
    AutoPlayStop(); Load();
    if(!windowHandle) CreateSettings();
    if(!windowHandle) return;
    Populate();ShowWindow(windowHandle,SW_SHOW);SetForegroundWindow(windowHandle);
}
void AutoPlayStart()
{
    Load();
    options=GetActionOptions();
    if(!Hero || !Validate()) return;
    AutoPlayStop();
    if(IsDead(Hero)||Hero->viewtype==VIEWTYPE_GHOST_||Hero->viewtype==VIEWTYPE_OBSERVE_) {Status(L"当前角色状态无法挂机");return;}
    running=true;negotiating=true;mapAtStart=MapNumber;actorAtStart=Hero->id;
    centerX=Hero->x;centerY=Hero->y;nextRow=0;
    for (int index=0;index<RowCount;++index) due[index]=g_ClientTime;
    memset(alternateTarget,0,sizeof(alternateTarget));
    itemDeadline=weaponDeadline=cpDeadline=g_ClientTime;
    Row probe;Query(ProbeRow,probe,Hero,g_ClientTime);Status(L"等待服务器确认挂机规则");
}
void AutoPlayStop()
{
    running=negotiating=preparing=executing=replyReady=magicReady=false;
    ticket.pending=false;activeRow=ProbeRow;ResetExtras();
    Status(L"挂机已停止");
}
bool AutoPlayIsRunning(){return running;}
void AutoPlayReceive(const void* data,size_t size)
{
    if(!running || !data || size!=sizeof(DragonAutoPlay::Response)) return;
    DragonAutoPlay::Response response={};memcpy(&response,data,sizeof(response));
    if(!ticket.Accept(response)) return;
    reply=response;replyReady=true;
}
void AutoPlayMagicPrepared(unsigned skill)
{
    if(running&&preparing&&pendingSkill==static_cast<int>(skill)&&Hero&&Hero->excute_MagicNum==pendingSkill&&Hero->ready_Magic==1) magicReady=true;
}
void AutoPlayMagicFinished(unsigned skill)
{
    if(running&&executing&&pendingSkill==static_cast<int>(skill))
    {executing=false;MarkAction(activeRow,g_ClientTime);}
}
void AutoPlayTick()
{
    if(!running) return;
    if(!Hero||Hero->id!=actorAtStart||MapNumber!=mapAtStart||ChangeMapFlag||IsDead(Hero)||Hero->viewtype==VIEWTYPE_GHOST_||Hero->viewtype==VIEWTYPE_OBSERVE_)
    {AutoPlayStop();return;}
    const uint32_t now=g_ClientTime;
    if(ticket.Expired(now)){AutoPlayStop();Status(L"服务器未响应挂机校验，已停止");return;}
    if(replyReady) ExecuteReply(now);
    if(!running || negotiating) return;
    UpdateExtras(true,config.hideMagic,config.hidePlayers,config.autoKick);
    if(preparing)
    {
        if(!RemainingMs(now,actionDeadline)){AutoPlayStop();Status(L"施法准备未完成，已停止");return;}
        if(magicReady && !RemainingMs(now,Hero->end_Casting))
        {
            CHARACTER* target=ReturnCharacterPoint(pendingTarget);
            if(!target||(target!=Hero&&(!Enemy(target)||!InConfiguredArea(target)))||Hero->excute_MagicNum!=pendingSkill)
            {AutoPlayStop();return;}
            preparing=false;executing=true;actionDeadline=now+5000;
            g_mgrBattle.SendMagicExecute(static_cast<BYTE>(pendingSkill),target,target->x,target->y);
        }
        return;
    }
    if(executing)
    {
        if(!RemainingMs(now,actionDeadline)){AutoPlayStop();Status(L"技能结果未返回，已停止");}
        return;
    }
    if(ticket.pending) return;
    if(!RemainingMs(now,itemDeadline))
    {
        const int current[3]={SCharacterData.nCharacterHP,SCharacterData.nCharacterMP,SCharacterData.nCharacterSP};
        const int maximum[3]={SCharacterData.nCharacterMAXHP,SCharacterData.nCharacterMAXMP,SCharacterData.nCharacterMAXSP};
        for(int resource=0;resource<3;++resource)
        {
            if(maximum[resource]<=0 || static_cast<int64_t>(current[resource])*100>=static_cast<int64_t>(maximum[resource])*config.thresholds[resource]) continue;
            bool sent=false;
            if(config.quickSlots[resource]>=0) sent=TryQuickRestore(resource,config.quickSlots[resource]);
            else for(int slot=0;slot<6 && !sent;++slot) sent=TryQuickRestore(resource,slot);
            if(sent){itemDeadline=now+10000;return;}
        }
    }
    if(config.switchWeapon&&!RemainingMs(now,weaponDeadline))
    {weaponDeadline=now+2000;if(TrySwitchWornWeapon())return;}
    if(RemainingMs(now,nextQuery)) return;
    if(config.autoCp && iCSPNow<25 && !RemainingMs(now,cpDeadline))
    {
        for(const ActionOption& option:options)
        {
            if(option.classification&CombatRecovery)
            {Row recovery;recovery.kind=option.kind;recovery.skill=option.skill;recovery.self=true;recovery.monster=false;recovery.expired=true;cpDeadline=now+2000;Query(RecoveryRow,recovery,Hero,now);return;}
        }
    }
    const int count=config.work?1:10;
    // Scheduled buffs/actions take priority; the single continuous row is the fallback.
    for(int pass=0;pass<2;++pass)
    for(int turn=0;turn<count;++turn)
    {
        const int index=config.work?10:(nextRow++%10);
        const Row& row=config.rows[index];
        if(!row.enabled || (row.expired && !config.renew) || row.continuous!=(pass==1) || RemainingMs(now,due[index])) continue;
        if(!IsLearned(row.kind,row.skill)){due[index]=now+5000;Status(L"部分配置技能不可用，请检查职业和已学技能");continue;}
        CHARACTER* target=RowTarget(row,index);
        if(!target){due[index]=now+500;continue;}
        Query(index,row,target,now);return;
    }
    nextQuery=now+250;
}
