#include "..\stdafx.h"
#include "AutoPlayServer.h"
#include "PlayerSessionSafety.h"
#include "../../Library/Shared/AutoPlayProtocol.h"
#include "../../Library/Shared/MagicPreparationGate.h"
#include "../../Library/Shared/BattleSkillSafety.h"
#include "SymbolItemMgr.h"

extern int CheckEventAttack();

namespace
{
using namespace DragonAutoPlay;
using DragonGameplay::IsPlayerSession;
using DragonGameplay::GetPlayerSessionIdentity;

struct QueryThrottle
{
    DWORD agent;
    DWORD user;
    DWORD character;
    DWORD last_ms;
    bool initialized;
};

QueryThrottle s_throttle[DRAGON_MAX_CONNECTIONS_ + 1] = {};
DragonGameplay::MagicPreparationGate s_preparation[DRAGON_MAX_CONNECTIONS_ + 1];





uint32_t SecondsRemaining(DWORD expires, DWORD now)
{
    if (expires <= now) return 0;
    const uint64_t milliseconds = uint64_t(expires - now) * 1000;
    return milliseconds > UINT32_MAX ? UINT32_MAX : uint32_t(milliseconds);
}

void Reject(Response& response, Reason reason)
{
    if (response.reason == Ready || reason == Dead ||
        (reason == War && response.reason != Dead))
        response.reason = uint8_t(reason);
    response.allowed = 0;
}

void AddWait(Response& response, uint32_t remaining)
{
    response.known |= WaitKnown;
    if (remaining > response.wait_ms) response.wait_ms = remaining;
    if (remaining != 0) Reject(response, Cooldown);
}

void QueryEffect(CHARLIST& target, const Request& request, Response& response)
{
    DWORD expires = 0;
    bool known = false;
    if (request.kind == Magic)
    {
        // Only slots with a proven, unambiguous authoritative producer.
        switch (request.skill)
        {
        case PRAY: expires = target.dwUpDivineMana; known = true; break;
        case AUTHORITY_LUCK: expires = target.dwDontMiss; known = true; break;
        case NULLITY_MAGIC: expires = target.dwMagicDamageToMana; known = true; break;
        case RECURRENCE_DIVINE: expires = target.dwReturnMagic; known = true; break;
        }
    }
    else if (request.kind == Combat)
    {
        switch (Magic_Ref[request.skill].Spell_Type)
        {
        case PASSIVE_COMBAT:
            known = true;
            if (target.GetPassiveCombat() == request.skill)
                expires = target.dwCombatState;
            break;
        case RECOVERY_COMBAT:
            known = true;
            if (target.GetRecoveryCombat() == request.skill)
                expires = target.GetRecoveryCombatTime();
            break;
        }
    }
    if (!known) return;
    response.known |= EffectKnown;
    response.effect_remaining_ms = SecondsRemaining(expires, g_curr_time);
    if (response.effect_remaining_ms != 0) response.known |= EffectActive;
}

bool IsLearnedMagic(const CHARLIST& actor, int skill)
{
    if (actor.Spell == WIZARD_SPELL)
        return DragonGameplay::HasLearnedSpell(skill, 0, actor.Ws);
    if (actor.Spell == PRIEST_SPELL)
        return DragonGameplay::HasLearnedSpell(skill, 150, actor.Ps);
    return false;
}

void QueryWork(CHARLIST& actor, const Request& request, Response& response)
{
    const int skill = request.skill;
    if (skill <= 0 || skill >= MAX_SKILLMAIN || !SkillMgr.CheckSkillNo(skill) ||
        SkillTbl[skill].NO != skill)
    {
        Reject(response, InvalidRequest);
        return;
    }
    response.known |= LearnedKnown | RulesKnown;
    response.target_type = uint8_t(SkillTbl[skill].Howto);
    if (!SkillMgr.CheckIsYourSkill(&actor, skill)) Reject(response, NotLearned);
    // dwSkillTime is the existing timeGetTime() request gate (Skill.cpp).
    const DWORD now = timeGetTime();
    const LONG difference = LONG(actor.dwSkillTime - now);
    AddWait(response, difference > 0 ? uint32_t(difference) + 1 : 0);
    if (actor.cur_skillno != 0)
    {
        uint32_t remaining = SecondsRemaining(actor.exetime_skill, g_curr_time);
        // Execution pending completion is still busy at the exact second.
        AddWait(response, remaining != 0 ? remaining : 250);
    }
    // Coordinates, materials, tools, terrain, and per-skill completion remain
    // with RecvSkillInfo/SkillMgr; no query performs SelectSkill or consumes.
}

void QueryBattle(CHARLIST& actor, CHARLIST& target,
    const Request& request, Response& response)
{
    const int skill = request.skill;
    const bool magic = request.kind == Magic;
    const bool valid = magic ? (skill >= LIFE_UP && skill <= RAGE_LIGHT_BALL) :
        (skill >= LIGHTNING_BOOM && skill <= WIND_EXTREME);
    if (!valid || skill >= Num_Of_Magic || Magic_Ref[skill].serial_Num == 0)
    {
        Reject(response, InvalidRequest);
        return;
    }

    response.known |= RulesKnown | LearnedKnown;
    response.skill_type = uint8_t(Magic_Ref[skill].magic_Type);
    response.target_type = uint8_t(Magic_Ref[skill].Appoint_Type);
    response.execute_type = uint8_t(Magic_Ref[skill].Excute_Type);
    const int class_index = actor.Class;
    if (class_index < WARRIOR || class_index > PRIEST ||
        (Magic_Ref[skill].nClass & (1 << class_index)) == 0)
        Reject(response, AbilityRequired);

    bool learned = false;
    if (magic) learned = IsLearnedMagic(actor, skill);
    else
    {
        const int level = actor.GetCombatLevel(skill);
        int required = 0;
        switch (level)
        {
        case 1: required = Magic_Ref[skill].basic_Level; break;
        case 2: required = Magic_Ref[skill].before_Magic; break;
        case 3: required = Magic_Ref[skill].basic_magery; break;
        }
        learned = level >= 1 && level <= 3;
        if (learned && actor.GetLevel() < required) Reject(response, AbilityRequired);
        const int tactics = actor.GetTacticsKind() - TACTICS_Crapple;
        if (tactics < 0 || tactics > TACTICS_Double_MaceFighting - TACTICS_Crapple ||
            (Magic_Ref[skill].nTactics & (1 << tactics)) == 0)
            Reject(response, EquipmentRequired);
        if (!actor.IsEnableCombat(BYTE(skill))) Reject(response, InsufficientResource);
    }
    if (!learned) Reject(response, NotLearned);

    // Combat persistent slots belong to the caster, not the attacked monster.
    if (magic || &target == &actor) QueryEffect(target, request, response);
    int range_tiles = Magic_Ref[skill].avail_Range;
    if (!magic && skill == CHARGING)
        range_tiles = learned ? actor.GetCombatValue(skill) : -1;
    if (range_tiles >= 0 && range_tiles <= UINT16_MAX / TILE_SIZE)
    {
        response.known |= RangeKnown;
        response.range_pixels = uint16_t(range_tiles * TILE_SIZE);
    }
    if (range_tiles > 0 && actor.GetDistance(target.X, target.Y) > range_tiles)
        Reject(response, OutOfRange);

    const int type_group = Magic_Ref[skill].magic_Type / 10;
    const bool harmful = type_group == 4 || type_group == 5;
    if (target.IsNpc() && !harmful) Reject(response, InvalidTarget);
    if (Magic_Ref[skill].magic_Type == MapInfo[MapNumber].nDisableMagicType)
        Reject(response, Disabled);
    if (Magic_Ref[skill].nDualStep > g_CSymbolMgr.GetSymbolGrade(&actor))
        Reject(response, EquipmentRequired);

    if (magic)
    {
        AddWait(response, SecondsRemaining(actor.GetCastingDeadline(), g_curr_time));
        const int prepare = actor.CalcMagicCastReady(BYTE(skill));
        // The legacy casting reply is WORD, retain exactly that wire value.
        response.prepare_ms = WORD(prepare);
        response.known |= PrepareKnown;
        const int cost = Magic_Ref[skill].exhaust_MP;
        if (cost >= 0 && cost <= UINT16_MAX)
        {
            response.known |= ManaCostKnown;
            response.mana_cost = uint16_t(cost);
        }
        if (actor.Mana < cost) Reject(response, InsufficientResource);
        if (actor.GetAbility(INT_) < Magic_Ref[skill].require_IT ||
            actor.GetAbility(WIS) < Magic_Ref[skill].require_WP ||
            actor.GetAbility(WSPS) < Magic_Ref[skill].point_WS)
            Reject(response, AbilityRequired);
        const int tactics = actor.Spell == WIZARD_SPELL ? TACTICS_Magery : TACTICS_Orison;
        if (actor.Skill[tactics] + 1 < Magic_Ref[skill].basic_magery)
            Reject(response, AbilityRequired);
        if (actor.dwCurse1 != 0 || !actor.IsCasting()) Reject(response, Disabled);
        if (harmful && (actor.IsPerfect() || actor.IsDivineUp()))
            Reject(response, Disabled);
    }
    else
    {
        // Recovery is the proven server-wide exclusion. No fabricated CD.
        AddWait(response, SecondsRemaining(actor.GetRecoveryCombatTime(), g_curr_time));
        if (actor.IsRecoveryCombatState()) Reject(response, Cooldown);
    }
}
}

void ResetAutoPlaySession(int connection)
{
    if (connection < DRAGON_CONNECTIONS_START || connection >= DRAGON_MAX_CONNECTIONS)
        return;
    s_preparation[connection].Reset();
}

void BeginPlayerMagicPreparation(int connection, unsigned char skill,
    unsigned short ready_ms)
{
    if (!IsPlayerSession(connection)) return;
    s_preparation[connection].Begin(GetPlayerSessionIdentity(connection), skill,
        GetTickCount64(), ready_ms);
}

bool CanExecutePlayerMagic(int connection, unsigned char skill)
{
    if (!IsPlayerSession(connection))
    {
        ResetAutoPlaySession(connection);
        return false;
    }
    return connections[connection].chrlst.GetMagic() == skill &&
        s_preparation[connection].CanExecute(GetPlayerSessionIdentity(connection), skill,
            GetTickCount64());
}

bool CanExecutePlayerMagicArrow(int connection, unsigned char skill)
{
    if (!IsPlayerSession(connection) ||
        (skill != FIRE_ARROW && skill != ICE_ARROW) ||
        connections[connection].chrlst.GetMagic() != skill ||
        Magic_Ref[skill].cast_Continue < 0) return false;
    // Match the client's activation window, measured after actual preparation.
    const uint64_t activeMs = uint64_t(Magic_Ref[skill].cast_Continue / 6) * 1000;
    return s_preparation[connection].CanExecuteWithin(GetPlayerSessionIdentity(connection),
        skill, GetTickCount64(), activeMs);
}

void RecvAutoPlayQuery(int connection, const void* payload, int payload_size)
{
    using namespace DragonAutoPlay;
    if (connection < 1 || connection > DRAGON_MAX_CONNECTIONS_ ||
        payload == NULL || payload_size != sizeof(Request)) return;
    t_connection& session = connections[connection];
    CHARLIST& actor = session.chrlst;
    if (!IsPlayerSession(connection)) return;

    QueryThrottle& throttle = s_throttle[connection];
    const DWORD now = timeGetTime();
    const bool same_identity = throttle.initialized &&
        throttle.agent == session.dwAgentConnectionIndex &&
        throttle.user == session.dwIDForAgent && throttle.character == actor.total_id;
    if (same_identity && DWORD(now - throttle.last_ms) < 200) return;
    throttle.agent = session.dwAgentConnectionIndex;
    throttle.user = session.dwIDForAgent;
    throttle.character = actor.total_id;
    throttle.last_ms = now;
    throttle.initialized = true;

    Request request;
    memcpy(&request, payload, sizeof(request));
    Response response = {};
    response.version = Version;
    response.kind = request.kind;
    response.skill = request.skill;
    response.target = request.target == 0 ? uint16_t(connection) : request.target;
    response.sequence = request.sequence;
    response.allowed = 1;
    if (request.version != Version || request.kind > Work)
        Reject(response, InvalidRequest);
    else
    {
        CHARLIST* target = GetCharListPtr(response.target);
        if (target == NULL || (target != &actor && (target->bAlive != ALIVE_ || !target->IsNpc() ||
            target->IsTamedNpc() || target->IsSummonMonster() ||
            target->Race == HUMAN || target->Race == SEALSTONE)))
            Reject(response, InvalidTarget);
        else
        {
            // Populate known facts even when a transient state blocks use.
            if (request.kind == Magic || request.kind == Combat)
                QueryBattle(actor, *target, request, response);
            else if (request.kind == Work) QueryWork(actor, request, response);
            else
            {
                if (request.skill != 0 || target == &actor)
                    Reject(response, InvalidRequest);
                const DWORD deadline = actor.GetAttackTime();
                const DWORD resume = deadline == MAXDWORD ? MAXDWORD : deadline + 1;
                AddWait(response, actor.GetAttackCount() >= 2 ?
                    SecondsRemaining(resume, g_curr_time) : 0);
                if (actor.dwDontAttack != 0) Reject(response, Disabled);
            }
            if (actor.IsDead() || actor.bAlive != ALIVE_ || target->IsDead())
                Reject(response, Dead);
            if (CheckEventAttack()) Reject(response, War);
            if (actor.IsStone() || actor.Hungry <= 0)
                Reject(response, Disabled);
            if ((request.kind == Magic && g_block_magic) ||
                ((request.kind == Attack || request.kind == Combat) && g_block_attack))
                Reject(response, Disabled);
        }
    }
    t_packet packet = {};
    static_assert(sizeof(packet.u) >= sizeof(response), "AutoPlay response exceeds packet");
    packet.h.header.type = ResponseCommand;
    packet.h.header.size = sizeof(response);
    memcpy(&packet.u, &response, sizeof(response));
    QueuePacket(connections, connection, &packet, 1);
}
