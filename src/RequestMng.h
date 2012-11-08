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
#include <httpfilt.h>
#include <list>
#include <string>

#include "HelpTools.h"
#include "RequestInfo.h"
#include "RequestQueue.h"
#include "config.h"
#include "log.h"

#ifndef __RequestMng_H__
#define __RequestMng_H__

#define MAX_POOL_SIZE	1024

using namespace std;

template <class T, int max_pool_size = MAX_POOL_SIZE> 
class SmallObjPool
{
private:
	list<T*> pool;
	CriticalSection cs;
public:
	SmallObjPool()
	{
	}
	~SmallObjPool()
	{
		for(list<T*>::iterator itor = pool.begin(); itor != pool.end(); itor++)
		{
			T* pobj = *itor;
			delete pobj;
		}

		pool.clear();
	}
	T* GetObj()
	{
		T* pobj;
		AutoLockScope lock_scope(cs);
		
		if(pool.size() > 0)
		{
			pobj = pool.front();
			pool.pop_front();
		}
		else
		{
			pobj = new(nothrow) T;
		}

		return pobj;
	}
	void ReturnObj(T* pobj)
	{
		AutoLockScope lock_scope(cs);
		
		if(pool.size() > max_pool_size)
		{
			delete pobj;
		}
		else
		{
			pool.push_back(pobj);
		}

	}
};

class RequestMng
{
private:
	//Request queue
	RequestQueue request_queue_obj;
	//
	IisRequestBarrierConfig config_obj;
	IisRequestBarrierLog log_obj;
	//small object pool
	SmallObjPool<RequestInfo> req_pool;
	SmallObjPool<RequestInfo2> req2_pool;
	volatile bool is_prologue_execute;
private:
	RequestMng();

public:
	SmallObjPool<RequestInfo>& GetRequestInfoPool()
	{
		return req_pool;
	}

	SmallObjPool<RequestInfo2>& GetRequestInfo2Pool()
	{
		return req2_pool;
	}
	
	IisRequestBarrierConfig& GetConfig()
	{
		return config_obj;
	}

	RequestQueue& GetRequestQueue()
	{
		return request_queue_obj;
	}

	IisRequestBarrierLog& GetLog()
	{
		return log_obj;
	}
public:
	//Initialize a 
	void Prologue();
	//Singleton accessor
	static RequestMng* GetInstance();
	// Set and Get HttpFilterContext->pFilterContext
	static RequestInfo2* GetCurrentRequestInfo(HTTP_FILTER_CONTEXT *pfc);
	// Cleanup HttpFilterContext->pFilterContext
	static void CleanupCurrentRequestInfo(HTTP_FILTER_CONTEXT *pfc);
	//check a ip is forbidden
	static bool isForbiddenIp(const RequestInfo& info);
	//
	static bool isUrlMatch(const RequestInfo2& info);
	//
	static bool RegisterRequest(const RequestInfo2& info);
};


#endif //__RequestMng_H__
