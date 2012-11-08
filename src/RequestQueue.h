/*
 * IisRequestBarrier 's core source code.
 * 2012.11.08: Create By Yin Mingjun - email: yinmingjuncn@gmail.com
 * 
 * Copyright 2012,  Yin MingJun - email: yinmingjuncn@gmail.com
 * Dual licensed under the MIT or GPL Version 2 licenses.
 * http://jquery.org/license
 *
 */


#include <list>
#include "RequestInfo.h"
#include "RequestQueueStat.h"
#include "HelpTools.h"

using namespace std;


#ifndef __RequestQueue_H__
#define __RequestQueue_H__

class IisRequestBarrierConfig;
class RequestMng;

struct ForbiddenInfo
{
	string ip;
	DWORD dwStartTime;
	ForbiddenInfo(const RequestInfo& info)
	{
		ip = info.client_ip;
		dwStartTime = info.dwTimeTicks;
	}
};

class ForbiddenIpQueue
{
private:
	typedef list<ForbiddenInfo> __ForbiddenList;
	__ForbiddenList forbidden_list;
	//DWORD forbidden_time_out;
	IisRequestBarrierConfig* config_obj;
public:
	ForbiddenIpQueue(IisRequestBarrierConfig* config_obj);
	void AddForbiddenIp(const RequestInfo& info);
	bool isForbiddenIp(const RequestInfo& info);
};

class RequestMng;

class RequestQueue
{
private:
	typedef list<RequestInfo*>	__RequestInfoList;

	ForbiddenIpQueue forbidden_queue_obj;
	RequestQueueStat queue_stat_obj;
	IisRequestBarrierConfig* config_obj;
	RequestMng* request_mng_obj;
	__RequestInfoList request_list;
	CriticalSection cs;
public:
	RequestQueue(RequestMng* request_mng_obj, IisRequestBarrierConfig* config_obj);
	virtual ~RequestQueue();
private:
	void remove_request(const RequestInfo& info);
	void adjust_request_queue(const RequestInfo& info);
	void enqueue_request(const RequestInfo& info);
public:
	// Register once per-request
	// return true, IP can pass to IIS
	// return false, this IP is forbidden
	bool RegisterRequest(const RequestInfo& info);
	// query a ip is forbidden.
	bool isForbiddenIp(const RequestInfo& info);
	bool isUrlMatch(const RequestInfo2& info);
};

#endif //__RequestQueue_H__