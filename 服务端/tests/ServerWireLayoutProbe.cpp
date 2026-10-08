#include "../Mapserver/stdafx.h"
int wireLayout[] = {
    sizeof(t_server_user_db_data), offsetof(t_server_user_db_data, nation),
    sizeof(NW_Character), sizeof(t_char_info_tactic),
    offsetof(t_char_info_tactic, dwExperience)
};
