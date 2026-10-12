// WarStartInfo.cpp: implementation of the CWarStartInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "..\stdafx.h"
#include "WarStartInfo.h"
#include "WarSchedule.h"

///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
///////////////////////////////////////////////////////////////////////////////

CWarStartInfo::CWarStartInfo()
{
	m_vtTime.clear();
	m_dwRemainTime = 0;
	m_nIndex = -1;
	m_bActive = false;
}

CWarStartInfo::~CWarStartInfo()
{
	for_each(m_vtTime.begin(), m_vtTime.end(), TDelete());
	m_vtTime.clear();
}

///////////////////////////////////////////////////////////////////////////////
// Public Method
///////////////////////////////////////////////////////////////////////////////

void CWarStartInfo::Register(START_UP_INFO* pTime)
{
	m_vtTime.push_back(pTime);
}

void CWarStartInfo::Execute()
{
    m_dwRemainTime = 0;
    m_nIndex = -1;
    m_bActive = false;
    for (size_t i = 0; i < m_vtTime.size(); ++i)
    {
        const START_UP_INFO& time = *m_vtTime[i];
        const DWORD remaining = GetWeeklyWarRemainingSeconds(
            time.nApplyDay, time.nStartHour, time.nStartMinute,
            time.nEndHour, time.nEndMinute, g_wday, g_hour, g_min, g_sec);
        if (remaining > m_dwRemainTime)
        {
            m_dwRemainTime = remaining;
            m_nIndex = static_cast<int>(i);
            m_bActive = true;
        }
    }
}
