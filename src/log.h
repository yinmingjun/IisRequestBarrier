/*
 * IisRequestBarrier 's core source code.
 * 2012.11.08: Create By Yin Mingjun - email: yinmingjuncn@gmail.com
 * 
 * Copyright 2012,  Yin MingJun - email: yinmingjuncn@gmail.com
 * Dual licensed under the MIT or GPL Version 2 licenses.
 * http://jquery.org/license
 *
 */


#include <stdio.h>
#include <windows.h>
#include "config.h"

#ifndef __DebugTrace_H__
#define __DebugTrace_H__

#define LOG_TEXT(__X__)		RequestMng::GetInstance()->GetLog().LogText##__X__
#define LOG_TEXT_WITH_TIME(__X__)		RequestMng::GetInstance()->GetLog().LogTextWithTime##__X__


class IisRequestBarrierLog
{
private:
	IisRequestBarrierConfig* config_obj;
	FILE* log_fh;
public:
	IisRequestBarrierLog(IisRequestBarrierConfig* config_obj);
	~IisRequestBarrierLog();
private:
	void LogTextArgs(char* szFormat, va_list args);
	void WriteLogFile(char* buf, int length);
public:
	void LogText(char*szFormat, ...);
	void LogTextWithTime(char*szFormat, ...);
};

#endif //__DebugTrace_H__
