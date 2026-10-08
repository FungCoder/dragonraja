// WarStartInfo.cpp: implementation of the CWarStartInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "..\stdafx.h"
#include "WarStartInfo.h"

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
	if (m_bActive)
	{
		++m_dwRemainTime;

		if (IsCheckOut(m_nIndex))
		{
			m_bActive = false;
		}
	}
	else
	{
		for (int i = 0; i < m_vtTime.size(); ++i)
		{
			if (IsCheckIn(i))
			{
				m_nIndex = i;
				m_bActive = true;
				return;
			}
		}

		m_dwRemainTime = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Protected Method
///////////////////////////////////////////////////////////////////////////////

bool CWarStartInfo::IsCheckIn(int nIndex) const
{
	if (g_wday != m_vtTime[nIndex]->nApplyDay)
	{
		return false;
	}
	
	if (g_hour != m_vtTime[nIndex]->nStartHour)
	{
		return false;
	}
	
	if (g_min != m_vtTime[nIndex]->nStartMinute)
	{
		return false;
	}

	return true;
}

bool CWarStartInfo::IsCheckOut(int nIndex) const
{
	if (g_hour != m_vtTime[nIndex]->nEndHour)
	{
		return false;
	}
	
	if (g_min != m_vtTime[nIndex]->nEndMinute)
	{
		return false;
	}

	return true;
}