#include "../stdafx.h"
#include "../dragon.h"
#ifdef WITH_MAP_FIRST
#include "../map.h"
#endif
#include "../Menu.h"
#include "../MenuSet.h"
#include "../Network.h"
int menuLayout[] = {
    sizeof(CMenuVariable), offsetof(CMenuVariable, m_GuildMemberName),
    sizeof(CGuildMemberName), sizeof(CNameOfGuildMemberList)
};
int actorLayout[] = {
    sizeof(CCharacter), offsetof(CCharacter, spell),
    offsetof(CCharacter, class_type), offsetof(CCharacter, name_status),
    sizeof(SMENU), offsetof(SMENU, nField), sizeof(SFIELD),
    offsetof(CCharacter, aStepInfo)
};
static_assert(sizeof(CMenuVariable) == 796, "Menu ABI regression");
int wireLayout[] = {
    sizeof(t_server_user_db_data), offsetof(t_server_user_db_data, nation),
    sizeof(NW_Character), sizeof(t_char_info_tactic),
    offsetof(t_char_info_tactic, dwExperience)
};
