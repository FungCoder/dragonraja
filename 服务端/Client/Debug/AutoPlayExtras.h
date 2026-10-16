#ifndef DRAGON_AUTOPLAY_EXTRAS_H
#define DRAGON_AUTOPLAY_EXTRAS_H

class CCharacter;

namespace DragonAutoPlayClient
{
    // These switches affect drawing only while auto play is running.
    void UpdateExtras(bool running, bool hideMagic, bool hidePlayers, bool autoKick);
    void ResetExtras();
    bool HideMagicEffects();
    bool HidePlayer(const CCharacter* character);
    bool SupportsAutoKick();
}

#endif
