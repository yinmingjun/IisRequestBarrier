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
#include <string>

using namespace std;

#ifndef __RequestInfo_H__
#define __RequestInfo_H__

struct RequestInfo
{
	DWORD dwTimeTicks;
	string  client_ip;
};

struct RequestInfo2: public RequestInfo
{
	string host;
	string method;
	string url;
};

#endif //__RequestInfo_H__
