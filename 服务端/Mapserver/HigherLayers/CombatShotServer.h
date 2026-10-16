#pragma once

void ResetCombatShotSession(int connection);
void CancelCombatShots(int connection);
void RecvCombatShotHello(int connection, const void* data, int size);
void RecvCombatShotHit(int connection, const void* data, int size);
bool UsesVersionedThrow(int connection);
bool IssueCombatShot(int connection, unsigned short target, short x, short y);
void RegisterLegacyThrowRelease(int connection);
bool AllowLegacyThrowResult(int connection);
