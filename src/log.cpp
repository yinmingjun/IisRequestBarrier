/*
 * IisRequestBarrier 's core source code.
 * 2012.11.08: Create By Yin Mingjun - email: yinmingjuncn@gmail.com
 * 
 * Copyright 2012,  Yin MingJun - email: yinmingjuncn@gmail.com
 * Dual licensed under the MIT or GPL Version 2 licenses.
 * http://jquery.org/license
 *
 */


#include "log.h"
#include <stdlib.h>

#define MAX_DEBUG_OUTPUT    2048

//class IisRequestBarrierLog
IisRequestBarrierLog::IisRequestBarrierLog(IisRequestBarrierConfig* config_obj)
	:config_obj(config_obj)
{
	log_fh = fopen(IisRequestBarrierConfig::GetLogFileName().c_str(), "ab");

	if(log_fh == NULL)
	{
#if _DEBUG
		char buf[MAX_PATH+1];
		_snprintf(buf, MAX_PATH, "[IisRequestBarrier] Fail to open log file: \"%s\"!\r\n", IisRequestBarrierConfig::GetLogFileName().c_str());
		buf[MAX_PATH] = '\0';
		OutputDebugStringA(buf);
#endif
		return;
	}
}

IisRequestBarrierLog::~IisRequestBarrierLog()
{
	fclose(log_fh);
	log_fh = NULL;
}

void IisRequestBarrierLog::LogTextArgs(char* szFormat, va_list args)
{
    char    szOutput[MAX_DEBUG_OUTPUT];
    int     nWritten = 0;

    nWritten = _vsnprintf( szOutput + nWritten,
                           MAX_DEBUG_OUTPUT,
                           szFormat,
                           args );

    if ( nWritten == -1 )
    {
        if ( MAX_DEBUG_OUTPUT < 3 )
        {
            return;
        }

        szOutput[MAX_DEBUG_OUTPUT - 3] = '\r';
        szOutput[MAX_DEBUG_OUTPUT - 2] = '\n';
        szOutput[MAX_DEBUG_OUTPUT - 1] = '\0';

		nWritten = MAX_DEBUG_OUTPUT-1;
    }

	WriteLogFile(szOutput, nWritten);
}

void IisRequestBarrierLog::LogTextWithTime(char*szFormat, ...)
{
    char    szOutput[MAX_DEBUG_OUTPUT];
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	sprintf(szOutput, "[%02d:%02d:%02d(%03d)] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	strcat(szOutput, szFormat);

    va_list args;

    va_start( args, szFormat );

    LogTextArgs( szOutput, args );

    va_end( args );
}

void IisRequestBarrierLog::LogText(char*szFormat, ...)
{
    va_list args;

    va_start( args, szFormat );

    LogTextArgs( szFormat, args );

    va_end( args );
}

void IisRequestBarrierLog::WriteLogFile(char* buf, int length)
{

	if(log_fh)
	{
		fwrite(buf, sizeof(char), length, log_fh);

		fflush(log_fh);
	}

}
