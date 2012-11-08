
#include "RequestMng.h"

#define SERVER_VALUE_MAX_LEN		1024

void RequestMng::Prologue()
{
	if(!is_prologue_execute)
	{
		is_prologue_execute = true;
	}
}

RequestMng* RequestMng::GetInstance()
{
	static RequestMng g_instance;
	return &g_instance;
}

// Helper function, to get a server variable.
char* GetServerVariable(HTTP_FILTER_CONTEXT *pfc, char* buffer, DWORD maxBufferSize, const char* variable)
{
	DWORD dwVariableSize = maxBufferSize;

	if( pfc->GetServerVariable(pfc, (LPSTR)variable, buffer, &dwVariableSize) )
	{
		buffer[dwVariableSize] = '\0';
	}
	
	return buffer;
}

RequestInfo2* RequestMng::GetCurrentRequestInfo(HTTP_FILTER_CONTEXT *pfc)
{
/*
	char verb[30] = "ALL_HTTP";
	char buf[10240];
	GetServerVariable(pfc, buf, 10240, verb);
*/
	_ASSERT( pfc && (pfc->GetServerVariable) );

	if( pfc->pFilterContext )
	{
		return (RequestInfo2*)pfc->pFilterContext;
	}

	RequestMng* reqMng = RequestMng::GetInstance();
	RequestInfo2* reqObj = NULL;
	char strValue[SERVER_VALUE_MAX_LEN+1];

	_ASSERT( pfc && (pfc->GetServerVariable) );
	
	reqObj = reqMng->GetRequestInfo2Pool().GetObj();

	reqObj->dwTimeTicks = GetTickCount();

	//"HTTP_HOST" "HTTP_METHOD" "REMOTE_ADDR" "HTTP_URL"
	//Convert all string into lower case
	reqObj->host = strlwr(GetServerVariable(pfc, strValue, SERVER_VALUE_MAX_LEN, "HTTP_HOST"));
	reqObj->method = strlwr(GetServerVariable(pfc, strValue, SERVER_VALUE_MAX_LEN, "HTTP_METHOD"));
	reqObj->client_ip = strlwr(GetServerVariable(pfc, strValue, SERVER_VALUE_MAX_LEN, "REMOTE_ADDR"));
	reqObj->url = strlwr(GetServerVariable(pfc, strValue, SERVER_VALUE_MAX_LEN, "HTTP_URL"));

	// Set request filter context
	pfc->pFilterContext = reqObj;

	return reqObj;
}


void RequestMng::CleanupCurrentRequestInfo(HTTP_FILTER_CONTEXT *pfc)
{
	_ASSERT( pfc && (pfc->GetServerVariable) );

	if( pfc->pFilterContext )
	{
		RequestMng* reqMng = RequestMng::GetInstance();
		RequestInfo2* reqObj = (RequestInfo2*)pfc->pFilterContext;
		reqMng->GetRequestInfo2Pool().ReturnObj(reqObj);
		pfc->pFilterContext = NULL;

	}

}

RequestMng::RequestMng()
	:request_queue_obj(this, &this->config_obj),
	log_obj(&this->config_obj),
	is_prologue_execute(false)
{
}


bool RequestMng::isForbiddenIp(const RequestInfo& info)
{
	return GetInstance()->GetRequestQueue().isForbiddenIp(info);
}

bool RequestMng::isUrlMatch(const RequestInfo2& info)
{
	return GetInstance()->GetRequestQueue().isUrlMatch(info);
}

bool RequestMng::RegisterRequest(const RequestInfo2& info)
{
	return GetInstance()->GetRequestQueue().RegisterRequest(info);
}