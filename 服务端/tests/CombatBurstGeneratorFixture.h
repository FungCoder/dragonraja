#pragma once
// Real generator/AddEffect bodies are extracted at test time. Only graphics,
// allocation, resource tables and scene insertion are replaced here.
using LPCHARACTER = CharacterFixture*;
struct MAGICLIST
{
    uint64_t shot_action = 0, shot_batch = 0;
    LPCHARACTER lpChar_Own = nullptr, lpChar_Target = nullptr;
    int magicSerial = 0, magicNum = 0, magicSubNum = 0;
    int next_Effect = 0, x = 0, y = 0, tarx = 0, tary = 0;
    int sprite_Num = 0, spr_Speed = 0, spr_Height = 0, target_Height = 0;
    int dir = 0, s_X = 0, s_Y = 0, t_X = 0, t_Y = 0;
    POINT map_Offset = {};
    int trans_Type = 0, trans_Level = 0, object_Num = 0, fallow = 0;
    int Caster_id = 0, Target_id = 0, impect_Effect_Num = 0, link_Frame = 0;
    int end_Time = 0, sound_LoopCount = 0, flag = 0, next_Flag = 0, magic_Runtime = 0;
    bool firePattern = false, failed = false;
};
struct EFFECT_INFO
{
    LPCHARACTER pCaster, pTarget;
    int nType, nEffect, nSub, nDir, nSrcX, nSrcY, nDstX, nDstY;
    uint64_t shotAction;
};
struct EffectTableFixture
{
    int sprite_Num = 0, spr_Speed = 0, spr_Height = 0;
    int trans_Type = 0, trans_Level = 0, object_Num = 0, fallow = 0;
    int next_Effect = 0, impect_Effect_Num = 0, link_Frame = 0;
    int end_Time = 0, sound_loop = 0, sound_Num = 0, sound_Volume = 0;
    char sprite_Name[30] = {};
};
static EffectTableFixture g_lpET[700], g_lpET2[700];
static POINT g_Map = {};
static unsigned effSpr_Count[1] = {};
static struct { void* img = nullptr; } spriteFixture;
static struct { decltype(&spriteFixture) EffSpr = &spriteFixture; } Effspr[1];
static MAGICLIST* g_lpML = nullptr;
static std::vector<MAGICLIST> generated;
#define MemAlloc(pointer, size) pointer = new MAGICLIST()
static void PlayListAutoSounds(int, int, int, int) {}
static void LoadEffectSprite(int, const char*) {}
static void LoadEffectAnimation(int, const char*) {}
static void InsertMagicList(MAGICLIST**, MAGICLIST* effect)
{
    generated.push_back(*effect);
    delete effect;
}
class CBattle
{
public:
    void EffectShoot(MAGICLIST*, int);
    void EffectMulti(MAGICLIST*, int);
    void AddEffect(EFFECT_INFO*);
    // Geometry is unchanged and outside authorization tests.
    void Coordinates(double, int&, int&) {}
};
#include "CombatBurstGeneratorProduction.inc"
#undef MemAlloc
