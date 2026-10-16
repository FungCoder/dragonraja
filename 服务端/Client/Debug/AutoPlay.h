#pragma once
#include <stddef.h>

void AutoPlayShowSettings();
void AutoPlayStart();
void AutoPlayStop();
bool AutoPlayIsRunning();
void AutoPlayTick();
void AutoPlayReceive(const void* data, size_t size);
void AutoPlayMagicPrepared(unsigned skill);
void AutoPlayMagicFinished(unsigned skill);
