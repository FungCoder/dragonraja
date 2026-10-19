#pragma once
#include "../../Library/Shared/WallExecutionGate.h"

void ResetCombatShotSession(int connection);
void CancelCombatShots(int connection);
void RecvCombatShotHello(int connection, const void* data, int size);
void RecvCombatShotHit(int connection, const void* data, int size);
bool IssueCombatShot(int connection, unsigned short target, short x, short y);
bool IssueProjectileMagic(int connection, const void* data, int size);
bool RecordPaidWallMagicExecution(int connection, unsigned magic, int durationSeconds);
bool CanReportPaidWallMagic(int connection, unsigned magic);
