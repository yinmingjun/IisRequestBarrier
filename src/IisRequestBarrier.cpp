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
#include "RequestMng.h"
#include <ctime>

bool InitializeFilter()
{
	bool bRet = RequestMng::GetInstance()->GetConfig().LoadSetting();

	LOG_TEXT(( "###############################################\r\n"));
	
	if(bRet)
	{
		LOG_TEXT_WITH_TIME(( "IisRequestBarrier Load Config OK!\r\n"));
		LOG_TEXT(( "Config file setting:\r\n"));
		LOG_TEXT(( "\tIsSwitchOn: %d\r\n", RequestMng::GetInstance()->GetConfig().IsSwitchOn()));
		LOG_TEXT(( "\tUrlMatchPattern: \"%s\"\r\n", RequestMng::GetInstance()->GetConfig().GetUrlMatchPattern().c_str()));
		LOG_TEXT(( "\tRequestQueueLength: %d\r\n", RequestMng::GetInstance()->GetConfig().GetRequestQueueLength()));
		LOG_TEXT(( "\tRequestQueueDuration: %d (s)\r\n", RequestMng::GetInstance()->GetConfig().GetRequestQueueDuration()));
		LOG_TEXT(( "\tForbiddenBarrier: %d\r\n", RequestMng::GetInstance()->GetConfig().GetForbiddenBarrier()));
		LOG_TEXT(( "\tForbiddenDuration: %d (s)\r\n", RequestMng::GetInstance()->GetConfig().GetForbiddenDuration()));
	}
	else
	{
		LOG_TEXT_WITH_TIME(( "IisRequestBarrier Load Config FAIL!\r\n"));
	}

	return true;
}

BOOL WINAPI __stdcall GetFilterVersion(HTTP_FILTER_VERSION *pVer)
{
	InitializeFilter();

	/* Specify the types and order of notification */

	pVer->dwFlags = (SF_NOTIFY_NONSECURE_PORT | SF_NOTIFY_SECURE_PORT | SF_NOTIFY_PREPROC_HEADERS | SF_NOTIFY_LOG | SF_NOTIFY_END_OF_REQUEST | SF_NOTIFY_ORDER_DEFAULT );

	pVer->dwFilterVersion = HTTP_FILTER_REVISION;

	strcpy(pVer->lpszFilterDesc, "UFIDA IisRequestBarrier, Version 2.0");

	//DebugFilter();

	return TRUE;
}

DWORD WINAPI __stdcall HttpFilterProc(HTTP_FILTER_CONTEXT *pfc, DWORD NotificationType, VOID *pvData)
{
	RequestInfo2* pReq2 = NULL;

	switch (NotificationType)	{

		case SF_NOTIFY_PREPROC_HEADERS:
			/* Check the request IP */
			pReq2 = RequestMng::GetCurrentRequestInfo(pfc);

			// Disable all access from forbidden IP
			if(RequestMng::isForbiddenIp(*pReq2))
			{
				RequestMng::CleanupCurrentRequestInfo(pfc);
				return SF_STATUS_REQ_FINISHED;
			}

			// check pattern, skip unmatch url
			if(RequestMng::isUrlMatch(*pReq2))
			{
				if(!RequestMng::RegisterRequest(*pReq2))
				{
					RequestMng::CleanupCurrentRequestInfo(pfc);
					return SF_STATUS_REQ_FINISHED;
				}
			}
			else
			{
				//ignore this request
			}

			break;
		case SF_NOTIFY_LOG :
#ifdef _DEBUG
			pReq2 = RequestMng::GetCurrentRequestInfo(pfc);
#endif
			RequestMng::CleanupCurrentRequestInfo(pfc);
			break;
		case SF_NOTIFY_END_OF_REQUEST:
			/* Remove this request out of queue. */
#ifdef _DEBUG
			pReq2 = RequestMng::GetCurrentRequestInfo(pfc);
#endif
			RequestMng::CleanupCurrentRequestInfo(pfc);
			break;
		default:
			break;				
	}
	
	//send to next filter
	return SF_STATUS_REQ_NEXT_NOTIFICATION;
}
