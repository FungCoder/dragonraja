// WheatherControl.h: interface for the CWheatherControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WHEATHERCONTROL_H__164EB5A2_DF0A_4DE3_8F1B_12320DBB32AA__INCLUDED_)
#define AFX_WHEATHERCONTROL_H__164EB5A2_DF0A_4DE3_8F1B_12320DBB32AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

const short int nMinimumLight = 9;
const short int nMaximumLight = 31;

class CWeatherControl  
{
public:
	CWeatherControl();
	virtual ~CWeatherControl();
public:
	DWORD dwCheckedTime;
	void CheckDayLight();
	void AutoSetDayLight(CHARLIST *ch);
	void SetDayLight(const short int nValue);
	void SetChLight(CHARLIST *ch,const int iLight,const int iTimeSec );
private:
	short int nDayLightControl;
};
extern CWeatherControl WeatherControl;

extern void CurrentGameDate( const DWORD t, int *y, int *mo, int *d, int *h, int *mi, int *sec );
extern int  LoadWeatherTable( void );
extern void CheckWeatherSystem( void );
extern short int getWeatherCorrection( void );

extern DWORD today_rainstart[5];
extern DWORD today_rainend[5];
extern int   today_rainamount[5];
extern int   today_temperature;
extern int	  today_weathercount;

#endif // !defined(AFX_WHEATHERCONTROL_H__164EB5A2_DF0A_4DE3_8F1B_12320DBB32AA__INCLUDED_)
