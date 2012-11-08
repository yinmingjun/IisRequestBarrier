/*
 * IisRequestBarrier 's core source code.
 * 2012.11.08: Create By Yin Mingjun - email: yinmingjuncn@gmail.com
 * 
 * Copyright 2012,  Yin MingJun - email: yinmingjuncn@gmail.com
 * Dual licensed under the MIT or GPL Version 2 licenses.
 * http://jquery.org/license
 *
 */


#include <windows.h>


#ifndef __HELP_TOOLS_H__
#define __HELP_TOOLS_H__

class CriticalSection
{
	CRITICAL_SECTION m_cs;
private:
	explicit CriticalSection(const CriticalSection& obj)
	{
	}
	CriticalSection& operator =(const CriticalSection& rhs)
	{
	}
public:
	CriticalSection()
	{
		InitializeCriticalSection(&m_cs);
	}
	~CriticalSection()
	{
		DeleteCriticalSection(&m_cs);
	}
	void lock()
	{
		EnterCriticalSection(&m_cs);
	}
	void unlock()
	{
		LeaveCriticalSection(&m_cs);
	}
};

class AutoLockScope
{
private:
	CriticalSection& cs;
public:
	AutoLockScope(CriticalSection& cs)
		:cs(cs)
	{
		cs.lock();
	}

	~AutoLockScope()
	{
		cs.unlock();
	}
};

#endif //__HELP_TOOLS_H__
