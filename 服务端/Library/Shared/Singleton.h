// Singleton.h: definition of the Singleton function.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SINGLETON_
#define _SINGLETON_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <cstdint>

///////////////////////////////////////////////////////////////////////////////
// BEGIN : Singleton ���� ������ ���� Ŭ����
template<typename T>
class TSingleton
{
public:
	TSingleton()
	{
		assert(s_pInstance == NULL);
		s_pInstance = (T*)((intptr_t)this + 
			               (intptr_t)(T*)1 - (intptr_t)(TSingleton<T>*)(T*)1);
	}
	
	virtual ~TSingleton() 
	{ 
		assert(s_pInstance != NULL);
		s_pInstance = NULL; 
	}
	
public:
	static T& Reference() 
	{ 
		assert(s_pInstance != NULL);
		return *s_pInstance; 
	}
	
	static T* Pointer()   
	{ 
		assert(s_pInstance != NULL);
		return s_pInstance; 
	}

private:
	static T* s_pInstance;
};
// END
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// BEGIN
template<typename T> T* TSingleton<T>::s_pInstance = NULL;
// END
///////////////////////////////////////////////////////////////////////////////
#endif