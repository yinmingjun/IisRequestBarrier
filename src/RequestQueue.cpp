/*
 * IisRequestBarrier 's core source code.
 * 2012.11.08: Create By Yin Mingjun - email: yinmingjuncn@gmail.com
 * 
 * Copyright 2012,  Yin MingJun - email: yinmingjuncn@gmail.com
 * Dual licensed under the MIT or GPL Version 2 licenses.
 * http://jquery.org/license
 *
 */


#include "RequestQueue.h"
#include "config.h"
#include "RequestMng.h"

DWORD TickCountDiff(DWORD from, DWORD to)
{
	if(from <= to)
		return (to - from);
	else
		return to + (UINT_MAX - from);
}

////class ForbiddenIpQueue
ForbiddenIpQueue::ForbiddenIpQueue(IisRequestBarrierConfig* config_obj)
	:config_obj(config_obj)
{
}

void ForbiddenIpQueue::AddForbiddenIp(const RequestInfo& info)
{
	//Check and update
	for(__ForbiddenList::iterator itor = forbidden_list.begin(); itor != forbidden_list.end(); )
	{
		//if(TickCountDiff(itor->dwStartTime, info.dwTimeTicks) > forbidden_time_out)
		if(TickCountDiff(itor->dwStartTime, info.dwTimeTicks) > (DWORD)(config_obj->GetForbiddenDuration()*1000))
		{
			//remove this forbidden info
			LOG_TEXT_WITH_TIME(("Forbidden IP [%s] is unfreezed!\r\n", itor->ip.c_str()));
			if(itor == forbidden_list.begin())
			{
				forbidden_list.erase(itor);
				itor = forbidden_list.begin();
			}
			else
			{
				__ForbiddenList::iterator itor2 = itor++; 
				forbidden_list.erase(itor2);
			}
		}
		else if(info.client_ip == itor->ip)
		{
			//Find forbidden ip, update request time, and skip it
			itor->dwStartTime = info.dwTimeTicks;
			return;
		}
		else
		{
			itor++;
		}
	}

	//add forbidden ip
	LOG_TEXT_WITH_TIME(("Forbidden IP [%s] is freezed!\r\n", info.client_ip.c_str()));
	forbidden_list.push_front(ForbiddenInfo(info));
}

bool ForbiddenIpQueue::isForbiddenIp(const RequestInfo& info)
{
	//Check and update
	for(__ForbiddenList::iterator itor = forbidden_list.begin(); itor != forbidden_list.end(); )
	{
		//if(TickCountDiff(itor->dwStartTime, info.dwTimeTicks) > forbidden_time_out)
		if(TickCountDiff(itor->dwStartTime, info.dwTimeTicks) > (DWORD)(config_obj->GetForbiddenDuration()*1000))
		{
			//remove this forbidden info
			LOG_TEXT_WITH_TIME(("Forbidden IP [%s] is unfreezed!\r\n", itor->ip.c_str()));
			if(itor == forbidden_list.begin())
			{
				forbidden_list.erase(itor);
				itor = forbidden_list.begin();
			}
			else
			{
				__ForbiddenList::iterator itor2 = itor++; 
				forbidden_list.erase(itor2);
			}
		}
		else if(info.client_ip == itor->ip)
		{
			//Find forbidden ip
			return true;
		}
		else
		{
			itor++;
		}
	}

	return false;
}

////class RequestQueue
RequestQueue::RequestQueue(RequestMng* request_mng_obj, IisRequestBarrierConfig* config_obj)
	:request_mng_obj(request_mng_obj),
	config_obj(config_obj),
	forbidden_queue_obj(config_obj)
{
}

RequestQueue::~RequestQueue()
{
	for(__RequestInfoList::iterator itor = request_list.begin(); itor != request_list.end(); itor++)
	{
		request_mng_obj->GetRequestInfoPool().ReturnObj(*itor);
	}
	request_list.clear();
}

void RequestQueue::remove_request(const RequestInfo& info)
{
	for(__RequestInfoList::iterator itor = request_list.begin(); itor != request_list.end(); )
	{
		if( (*itor)->client_ip == info.client_ip )
		{
			RequestInfo *pobj = *itor;

			//remove this request
			if(itor == request_list.begin() )
			{
				request_list.erase(itor);
				itor = request_list.begin();
			}
			else
			{
				__RequestInfoList::iterator itor2 = itor++;
				request_list.erase(itor2);
			}

			//return obj into pool
			request_mng_obj->GetRequestInfoPool().ReturnObj(pobj);
		}
		else
		{
			itor++;
		}
	}

	//update queue statistics
	queue_stat_obj.RemoveRequestStateInfo(info);
}

bool RequestQueue::isUrlMatch(const RequestInfo2& info)
{
	return (string::npos != info.url.find(request_mng_obj->GetConfig().GetUrlMatchPattern()));
}


void RequestQueue::adjust_request_queue(const RequestInfo& info)
{
	int remove_count = request_list.size() - this->request_mng_obj->GetConfig().GetRequestQueueLength();

	if( remove_count > 0 )
	{
		for(int i= 0;i<remove_count;i++)
		{
			RequestInfo* pobj = request_list.front();
			request_list.pop_front();
			queue_stat_obj.Dequeue(*pobj);
			request_mng_obj->GetRequestInfoPool().ReturnObj(pobj);
		}
	}
	
	DWORD dwNow = info.dwTimeTicks;

	while((request_list.begin() != request_list.end()) && TickCountDiff(request_list.front()->dwTimeTicks, dwNow) > (DWORD)(this->request_mng_obj->GetConfig().GetRequestQueueDuration()*1000))
	{
		RequestInfo* pobj = request_list.front();
		request_list.pop_front();
		queue_stat_obj.Dequeue(*pobj);
		request_mng_obj->GetRequestInfoPool().ReturnObj(pobj);
	}

}

void RequestQueue::enqueue_request(const RequestInfo& info)
{
	RequestInfo* pobj = request_mng_obj->GetRequestInfoPool().GetObj();
	*pobj = info;
	queue_stat_obj.Enqueue(info);
	request_list.push_back(pobj);
}

bool RequestQueue::RegisterRequest(const RequestInfo& info)
{
	AutoLockScope lock_scope(cs);

	//1. Check forbidden_queue
	if (forbidden_queue_obj.isForbiddenIp(info))
	{
		return false;
	}

	//2. Put request into queue, and update statistics info.
	//Check IP statistics
	//if( queue_stat_obj.GetStatInfo(info).dwRequestCount > dwForbiddenBarrier )
	if( queue_stat_obj.GetStatInfo(info).dwRequestCount >= (DWORD) this->request_mng_obj->GetConfig().GetForbiddenBarrier())
	{
		forbidden_queue_obj.AddForbiddenIp(info);
		remove_request(info);
		return false;
	}

	//Enqueue request
	enqueue_request(info);

	//Adjust queue
	adjust_request_queue(info);
	return true;
}

bool RequestQueue::isForbiddenIp(const RequestInfo& info)
{
	return forbidden_queue_obj.isForbiddenIp(info);
}
