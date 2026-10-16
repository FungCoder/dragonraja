#include "stdafx.h"
#include "dragon.h"
#include "AutoPlayExtras.h"

namespace
{
    bool g_hideMagic = false;
    bool g_hidePlayers = false;
}

namespace DragonAutoPlayClient
{
    void UpdateExtras(bool running, bool hideMagic, bool hidePlayers, bool autoKick)
    {
        g_hideMagic = running && hideMagic;
        g_hidePlayers = running && hidePlayers;
        // Party presence is map-local and the current protocol has no leader
        // authority. Never infer a kick from visibility or send a delete here.
        (void)autoKick;
    }

    void ResetExtras()
    {
        g_hideMagic = false;
        g_hidePlayers = false;
    }

    bool HideMagicEffects()
    {
        return g_hideMagic;
    }

    bool HidePlayer(const CCharacter* character)
    {
        return g_hidePlayers && Hero && character && character != Hero &&
            character->type == SPRITETYPE_CHARACTER;
    }

    bool SupportsAutoKick()
    {
        return false;
    }
}
