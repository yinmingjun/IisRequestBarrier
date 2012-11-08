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

using namespace std;

#ifndef __RequestQueueStat_H__
#define __RequestQueueStat_H__

struct RequestStat
{
	string ip;
	DWORD dwRequestCount;

	//function
	RequestStat()
	{
		dwRequestCount = 0;
	}
	RequestStat(const string& ip)
	{
		this->ip = ip;
		dwRequestCount = 1;
	}
};

class RequestQueueStat
{
	typedef list<RequestStat> RequestStatList;

	RequestStatList stat_list;
	RequestStat operation_context;
public:
	RequestQueueStat();
	virtual ~RequestQueueStat();
public:
	const RequestStat& Enqueue(const RequestInfo& info);
	const RequestStat& Dequeue(const RequestInfo& info);
	const RequestStat& GetStatInfo(const RequestInfo& info);
	bool RemoveRequestStateInfo(const RequestInfo& info);
};

#endif //__RequestQueueStat_H__