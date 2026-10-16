#pragma once
#include <stdint.h>
#include <stddef.h>

void CombatShotBegin();
void CombatShotReset();
void CombatShotTick();
void CombatShotReceive(unsigned command, const void* data, size_t size);
bool CombatShotReady();
bool CombatShotNeedsHandshake(int activeCombat, int bowType);
void CombatShotForgetPending();
uint64_t CombatShotTake(unsigned caster, unsigned target, int effect);
uint64_t CombatShotTakeBurst(unsigned caster, unsigned target, unsigned skill);
unsigned CombatShotBurstSkill(uint64_t batch);
unsigned CombatShotBurstCount(uint64_t batch, unsigned skill);
uint64_t CombatShotTakePart(uint64_t batch, unsigned part, unsigned count);
void CombatShotSendHit(uint64_t action, unsigned target, int x, int y, bool wanted);
