// ProtectionMagic.cpp: implementation of the CProtectionMagic class.
//
//////////////////////////////////////////////////////////////////////

#include "..\stdafx.h"
#include "ProtectionMagic.h"

#include "..\HigherLayers\OP_Magic.h"
///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
///////////////////////////////////////////////////////////////////////////////

CProtectionMagic::CProtectionMagic()
{
  
}

CProtectionMagic::~CProtectionMagic()
{

}

///////////////////////////////////////////////////////////////////////////////
// Protected Method
///////////////////////////////////////////////////////////////////////////////

bool CProtectionMagic::Bind()
{ 
	Resist(PROTECTION, &CProtectionMagic::Protection);
	Resist(SHIELD, &CProtectionMagic::Shield);
	Resist(FIRE_RESIST, &CProtectionMagic::FireResist);
	Resist(ICE_RESIST , &CProtectionMagic::IceResist);
	Resist(ELECT_RESIST, &CProtectionMagic::ElectResist);
	Resist(TWINKLE_STAR, &CProtectionMagic::TwinkleStar);
	Resist(PROTECT_RANGE_WEAPON, &CProtectionMagic::ProtectRangeWeapon); // ��Ÿ� ����κ��� ��ȣ
	Resist(DRAGON_SCALE, &CProtectionMagic::DragonScale);
	Resist(BLESS, &CProtectionMagic::Bless);
	Resist(PROTECT_DANGER, &CProtectionMagic::ProtectDanger);
	Resist(GREAT_SHIELD, &CProtectionMagic::GreatShield);
	Resist(AUTHORITY_LETHY, &CProtectionMagic::AuthorityLethy); // CSD-021015
	Resist(NULLITY_MAGIC, &CProtectionMagic::NullityMagic);
	Resist(CANCEL_DIVINE, &CProtectionMagic::CancelDivine);
	Resist(RECURRENCE_DIVINE, &CProtectionMagic::RecurrenceDivine);
	Resist(PERFECT_PROTECT, &CProtectionMagic::PerfectProtect);
	Resist(AUTHORITY_OF_ATHYAS, &CProtectionMagic::AuthorityOfAthyas); // 030415 kyo 
	return true;
} 

bool CProtectionMagic::DragonScale()
{ // [86]�巡���� ��� : ������ ���� ������ 60% ���
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ������ ���� 60% ����
	m_pTarget->SetPhysicalDefense(m_dwNow + wPeriod, 60, 60, 60);
	// �Ϲ� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}

bool CProtectionMagic::ProtectRangeWeapon()
{ // [79]��Ÿ� ����κ��� ��ȣ : ��Ÿ� ���� ������ 90% ���
	// �����ð� ���
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ��Ÿ� ���� ������ 90% ���
	m_pTarget->dpLong.SetState(m_dwNow + wPeriod, 90);
	// �Ϲ� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}

bool CProtectionMagic::FireResist()
{ // [28]�ҿ� ���� ���� : �Ұ迭 ���ݸ����� ���� ���� 50% ����
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// �Ұ迭 ���ݸ����� ���� ���� 50% ����
	m_pTarget->SetMagicalDefense(m_dwNow + wPeriod, 50, 0, 0, 0);
	// �Ϲ� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}

bool CProtectionMagic::IceResist()
{ // [37]���� ���� : �����迭 ���ݸ����� ���� ���� 50% ����
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// �����迭 ���ݸ����� ���� ���� 50% ����
	m_pTarget->SetMagicalDefense(m_dwNow + wPeriod, 0, 50, 0, 0);
	// CSD-021216 : �� ���� ���� ���
	m_pTarget->SetCurseDefense(m_dwNow + wPeriod, CMagicState::CURSE_FREEZE);
	// �Ϲ� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration= wPeriod;
	SendMagicResult();
	return true;
}

bool CProtectionMagic::ElectResist()
{ // [47]���ݿ� ���� ���� : ���ݰ迭 ���� ������ ���� ���� 50% ����
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ���ݰ迭 ���� ������ ���� ���� 50% ����
	m_pTarget->SetMagicalDefense(m_dwNow + wPeriod, 0, 0, 50, 0);
	// �Ϲ� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}

bool CProtectionMagic::TwinkleStar()
{ // [53]��ȣ�� : ������ ���� ������ 40% ���, ���� 1�迭�� ���� 100% ���
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ������ ���� 30% ����
	m_pTarget->SetPhysicalDefense(m_dwNow + wPeriod, 40, 40, 40);
	// ���� 1�迭 ���� ���
	m_pTarget->SetCurseDefense(m_dwNow + wPeriod, CMagicState::CURSE_1);
	// �Ϲ� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}

bool CProtectionMagic::Protection()
{ // [2]��ȣ : ������ ���� ������ 30% ���, ���� 1, 3, 5�迭�� ���� 100% ���
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ������ ���� 30% ����
	m_pTarget->SetPhysicalDefense(m_dwNow + wPeriod, 30, 30, 30);
	// ���� 3, 4, 5�迭 ���� ���
	m_pTarget->SetCurseDefense(m_dwNow + wPeriod, 
		CMagicState::CURSE_1|CMagicState::CURSE_3|CMagicState::CURSE_4);
	// �Ϲ� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}

bool CProtectionMagic::Shield()
{ // [3]��ȣ�� : ������ ���� ������, ��, ����, ���ݰ迭�� ���ݸ��� 40% ���
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ������ ���� 40% ����
	m_pTarget->SetPhysicalDefense(m_dwNow + wPeriod, 40, 40, 40);
	// ��, ����, ���ݰ迭 ���ݸ��� ���� 40 ����
	m_pTarget->SetMagicalDefense(m_dwNow + wPeriod, 40, 40, 40, 0);
	// �Ϲ� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}

bool CProtectionMagic::Bless()
{ // [153]���� �ູ : ���ְ迭(1, 2, 3, 4) ������ 100% ���
	// �����ð� ���
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ���� 1, 2, 3, 4�迭 ���� ���
	m_pTarget->SetCurseDefense(m_dwNow + wPeriod, 
		CMagicState::CURSE_1|CMagicState::CURSE_2|CMagicState::CURSE_3|CMagicState::CURSE_4);
	// �Ϲ� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}

bool CProtectionMagic::ProtectDanger()
{	// [152]�������κ��� ��ȣ : ������ ���� ������ 40 ~ 50% ���, ���ְ迭(1, 2) ���
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ���� ���
	const int nWis = m_pCaster->GetAbility(WIS) - GetRequireWidom(m_nIndex);
	const int nTactics = m_pCaster->Skill[TACTICS_Orison] - GetRequireTactics(m_nIndex);
	int nBasic = (nWis + nTactics)/10 + 40;
	if (nBasic < 0)   nBasic = 0;
	if (nBasic > 50)  nBasic = 50;
	m_pTarget->SetPhysicalDefense(m_dwNow + wPeriod, nBasic, nBasic, nBasic);
	// ���� 1, 2�迭 ���� ���
	m_pTarget->SetCurseDefense(m_dwNow + wPeriod, 
		CMagicState::CURSE_1|CMagicState::CURSE_2);
	// �Ϲ� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}

bool CProtectionMagic::GreatShield()
{ // [158]������ �⵵
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ���� ���
	const int nWis = m_pCaster->GetAbility(WIS) - GetRequireWidom(m_nIndex);
	const int nTactics = m_pCaster->Skill[TACTICS_Orison] - GetRequireTactics(m_nIndex);
	int nBasic = (nWis + nTactics)/10 + 40;
	if (nBasic < 0)   nBasic = 0;
	if (nBasic > 50)  nBasic = 50;
	m_pTarget->SetPhysicalDefense(m_dwNow + wPeriod, nBasic, nBasic, nBasic);
	// ���� 1, 2�迭 ���� ���
	m_pTarget->SetCurseDefense(m_dwNow + wPeriod, 
		CMagicState::CURSE_1|CMagicState::CURSE_2);
	// �Ϲ� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}

bool CProtectionMagic::AuthorityLethy()
{ //< CSD-021019 : [184]��Ƽ�� �Ǵ�
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ������ ���� ���� ���
	const int nWis = m_pCaster->GetAbility(WIS) - GetRequireWidom(m_nIndex);
	const int nTactics = m_pCaster->Skill[TACTICS_Orison] - GetRequireTactics(m_nIndex);
	int nBasic = (nWis + nTactics)/10;
	if (nBasic < 0)   nBasic = 0;
	if (nBasic > 10)  nBasic = 10;
	// ���� ���� ���� 40 - 50% ����
	const int nStrike = 40 + nBasic;
	m_pTarget->SetPhysicalDefense(m_dwNow + wPeriod, nStrike, nStrike, nStrike);
	// ���� ���� ���� 20 - 30% ����
	const int nMagic = 20 + nBasic;
	m_pTarget->SetMagicalDefense(m_dwNow + wPeriod, nMagic, nMagic, nMagic, nMagic);
	// �Ϲ� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	// ����, ���� ���� ������ �ݻ� �������� ����
	m_pTarget->dwStrikeReflect = m_dwNow + wPeriod;
	m_pTarget->dwMagicReflect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
} //> CSD-021019

bool CProtectionMagic::NullityMagic()
{ // [24]������ȿȭ : ���� �ð����� ���ݰ迭 ������ 100% ���(��������ŭ ���� �Һ�)
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ���ݰ迭 ������ 100% ���(��������ŭ ���� �Һ�)
	m_pTarget->dwMagicDamageToMana = m_dwNow + wPeriod;
	// Ư�� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwSpecialProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}

bool CProtectionMagic::CancelDivine()
{ // [159]�ݽŷ� ��ȿȭ : ���ְ迭(1, 2, 3, 4, 5)  ������ 100% ���
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ���� 1, 2, 3, 4, 5�迭 ���� ���
	m_pTarget->SetCurseDefense(m_dwNow + wPeriod, 
		CMagicState::CURSE_1|CMagicState::CURSE_2|CMagicState::CURSE_3|CMagicState::CURSE_4|CMagicState::CURSE_5);
	// Ư�� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}

bool CProtectionMagic::RecurrenceDivine()
{ // [160]ȸ���� �Ǵ� : ���� 1, 2, 3, 4�迭 ������ �����ڿ��� �ǵ���
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ���ְ迭(1, 2, 3, 4) ������ �����ڿ��� �ǵ���
	m_pCaster->dwReturnMagic = m_dwNow + wPeriod;
	// Ư�� ��ȣ�迭 ���� �������� ����
	m_pCaster->dwSpecialProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}

bool CProtectionMagic::PerfectProtect()
{	// [157]������ ��� : ��� ���� ���� ������ ����(Tactic�� 40���ʹ� 100%)�� ���� 
	//                    �������ڴ� ȸ���迭, ��ȣ�迭, �����迭 �������� ����� �� ����
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ������ ����� ���
	const int nTactics = m_pCaster->Skill[TACTICS_Orison];
	int nBasic = 80 + (nTactics>>1);
	if (nBasic > 100)  nBasic = 100;
	// ������ ����
	m_pTarget->SetPhysicalDefense(m_dwNow + wPeriod, nBasic, nBasic, nBasic);
	m_pTarget->SetMagicalDefense(m_dwNow + wPeriod, nBasic, nBasic, nBasic, nBasic);
	m_pTarget->SetCurseDefense(m_dwNow + wPeriod, 
		CMagicState::CURSE_1|CMagicState::CURSE_2|
		CMagicState::CURSE_3|CMagicState::CURSE_4|
		CMagicState::CURSE_5);
	//  ���� ��ȣ �迭 ������ �Ϲ��̸鼭 Ư�� ��ȣ�迭 ���� ���·� ����
	m_pTarget->dwExceptProtect = m_dwNow + wPeriod;
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	m_pTarget->dwSpecialProtect = m_dwNow + wPeriod;
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}
bool CProtectionMagic::AuthorityOfAthyas() // 030415 kyo 
{ // [185] �ƻ����� �Ǵ� �������20, ���ݸ������50, �Ϲ� ����,�̵��ӵ� 30%����
	// ��Ƽ�� �Ǵ� + ��ȣ + �ӵ�����
	const WORD wPeriod = CalcPeriod(); // �����ð� ���
	// ���� ���
	const int nWis = m_pCaster->GetAbility(WIS) - GetRequireWidom(m_nIndex);
	const int nTactics = m_pCaster->Skill[TACTICS_Orison] - GetRequireTactics(m_nIndex);
	int nBasic = (nWis + nTactics)/10;
	if (nBasic < 0)   nBasic = 0;
	if (nBasic > 10)  nBasic = 10;
	
	//������� 10 - 20
	const int nStrike = nBasic + 10;
	m_pTarget->SetPhysicalDefense(m_dwNow + wPeriod, nStrike, nStrike, nBasic);
	// ���� 1, 2�迭 ���� ���
	//m_pTarget->SetCurseDefense(m_dwNow + wPeriod, CMagicState::CURSE_1|CMagicState::CURSE_2);
	// ���� ���� ���� 40 - 50% ����
	const int nMagic = 40 + nBasic;
	m_pTarget->SetMagicalDefense(m_dwNow + wPeriod, nMagic, nMagic, nMagic, nMagic);
	// �Ϲ� ��ȣ�迭 ���� �������� ����
	m_pTarget->dwNormalProtect = m_dwNow + wPeriod;
	// �̵��ӵ� 20-30%����
	const int nSpeed = 20 + nBasic;
	m_pTarget->speedUp.SetState(m_dwNow + wPeriod, nSpeed);
	
	// �������� ����ġ ���
	::AddCasterEXP(m_pCaster, m_pTarget, 2, 0, m_nIndex);
	// ������� ����ڿ��� Packet���� ������
	InitMagicResult(CMD_MAGIC_RESULT_T);
	m_packet.u.magic.server_magic_result_t.nMagic = m_nIndex;
	m_packet.u.magic.server_magic_result_t.idTarget = m_idTarget;
	m_packet.u.magic.server_magic_result_t.nState = m_pTarget->GetState();
	m_packet.u.magic.server_magic_result_t.nResult = HIT_AND_NOTDEAD;
	m_packet.u.magic.server_magic_result_t.wDuration = wPeriod;
	SendMagicResult();
	return true;
}