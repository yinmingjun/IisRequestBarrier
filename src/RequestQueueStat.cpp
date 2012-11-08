/*
 * IisRequestBarrier 's core source code.
 * 2012.11.08: Create By Yin Mingjun - email: yinmingjuncn@gmail.com
 * 
 * Copyright 2012,  Yin MingJun - email: yinmingjuncn@gmail.com
 * Dual licensed under the MIT or GPL Version 2 licenses.
 * http://jquery.org/license
 *
 */


#include "RequestQueueStat.h"

//class RequestQueueStat
RequestQueueStat::RequestQueueStat()
{
}
RequestQueueStat::~RequestQueueStat()
{
}
const RequestStat& RequestQueueStat::Enqueue(const RequestInfo& info)
{
	//update
	for(RequestStatList::iterator itor = stat_list.begin(); itor != stat_list.end(); itor++)
	{
		if( (*itor).ip == info.client_ip)
		{
			(*itor).dwRequestCount++;
			operation_context = *itor;
			return operation_context;
		}
	}

	//Enqueue a new stat
	RequestStat stat(info.client_ip);
	stat_list.push_front(stat);
	operation_context = stat;
	return operation_context;
}

const RequestStat& RequestQueueStat::Dequeue(const RequestInfo& info)
{
	//update
	for(RequestStatList::iterator itor = stat_list.begin(); itor != stat_list.end(); itor++)
	{
		if( (*itor).ip == info.client_ip)
		{
			(*itor).dwRequestCount--;
			operation_context = (*itor);
			if((*itor).dwRequestCount == 0)
			{
				stat_list.erase(itor);
			}
			return operation_context;
		}
	}

	//???
	operation_context.ip = info.client_ip;
	operation_context.dwRequestCount = 0;

	return operation_context;
}

const RequestStat& RequestQueueStat::GetStatInfo(const RequestInfo& info)
{
	for(RequestStatList::iterator itor = stat_list.begin(); itor != stat_list.end(); itor++)
	{
		if( itor->ip == info.client_ip)
		{
			operation_context = *itor;
			return operation_context;
		}
	}

	//???
	operation_context.ip = info.client_ip;
	operation_context.dwRequestCount = 0;
	return operation_context;
}

bool RequestQueueStat::RemoveRequestStateInfo(const RequestInfo& info)
{
	for(RequestStatList::iterator itor = stat_list.begin(); itor != stat_list.end(); itor++)
	{
		if( (*itor).ip == info.client_ip)
		{
			stat_list.erase(itor);
			return true;
		}
	}

	return false;
}
