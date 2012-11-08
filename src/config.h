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
#include <cstdlib>
#include <string>

using namespace std;

#include "HelpTools.h"

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MODULE_NAME				"IisRequestBarrier.dll"

class IniSetting
{
protected:
	string ini_section_name;
protected:
	static bool GetCurrentModuleFileName(const string& mod_name, string& mod_file_name);
	static void TrimCommentAndTrailingWhitespace(CHAR *  szString);
	bool GetStringFromConfig(
		const string& ini_file, 
		const string section_name, 
		const string& szKey, 
		const string& szDefault, 
		string& strString);
	int GetIntFromConfig(
		const string& ini_file, 
		const string section_name, 
		const string& szKey, 
		int nDefault);
public:
	IniSetting();
	~IniSetting();
};


class IisRequestBarrierConfig: public IniSetting
{
private:
	bool is_base_config;
	string url_match_pattern;
	bool switch_flag;
	int request_queue_length;
	int request_queue_duration;
	int forbidden_duration;
	int forbidden_barrier;
public:
	IisRequestBarrierConfig();
	~IisRequestBarrierConfig();
private:
	void ConfigBaseClass();
public:
	static bool isSetFileName;
	static string ini_file_name;
	static string log_file_name;
	static void SetFileName();
	static const string& GetIniFileName();
	static const string& GetLogFileName();
public:
	bool LoadSetting();
	//Read config
	const string& GetUrlMatchPattern()
	{
		return url_match_pattern;
	}

	bool IsSwitchOn()
	{
		return switch_flag;
	}

	int GetRequestQueueLength()
	{
		return request_queue_length;
	}

	int GetRequestQueueDuration()
	{
		return request_queue_duration;
	}

	int GetForbiddenDuration()
	{
		return forbidden_duration;
	}

	int GetForbiddenBarrier()
	{
		return forbidden_barrier;
	}

};

#endif //__CONFIG_H__

