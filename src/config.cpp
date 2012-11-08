/*
 * IisRequestBarrier 's core source code.
 * 2012.11.08: Create By Yin Mingjun - email: yinmingjuncn@gmail.com
 * 
 * Copyright 2012,  Yin MingJun - email: yinmingjuncn@gmail.com
 * Dual licensed under the MIT or GPL Version 2 licenses.
 * http://jquery.org/license
 *
 */

#include "config.h"

#define MAX_CONFIG_LINE			1024

#define INI_SECTION_NAME		"Options"

#define DEFAULT_SWITCH_FLAG				(true)
#define DEFAULT_URL_MATCH_PATTERN		".aspx"
#define DEFAULT_REQUEST_QUEUE_LENGTH	1024
#define DEFAULT_REQUEST_QUEUE_DURATION	(1*60)	//1 minute
#define DEFAULT_FORBIDDEN_DURATION		(5*60)	//5 minute
//if ip requests in queue match times > FORBIDDEN_BARRIER, then upgrade it into forbidden queue.
#define DEFAULT_FORBIDDEN_BARRIER		30

//class IniSetting
IniSetting::IniSetting()
{
}

IniSetting::~IniSetting()
{
}

void IniSetting::TrimCommentAndTrailingWhitespace( CHAR *  szString )
{
    LPSTR   pCursor = szString;
    LPSTR   pWhite = NULL;

    while ( *pCursor )
    {
        if ( *pCursor == ';' )
        {
            *pCursor = '\0';
            break;
        }

        if ( *pCursor == ' ' || *pCursor == '\t' )
        {
            if ( !pWhite )
            {
                pWhite = pCursor;
            }
        }
        else
        {
            pWhite = NULL;
        }

        pCursor++;
    }

    if ( pWhite )
    {
        *pWhite = '\0';
    }

    return;
}

bool IniSetting::GetCurrentModuleFileName(const string& mod_name, string& mod_file_name)
{
	char buf[MAX_PATH+1] = "\0";

	if( 0 == GetModuleFileNameA(GetModuleHandle(mod_name.c_str()), buf, MAX_PATH) )
	{
		return false;
	}

	//replace .dll into .ini
	mod_file_name = _strlwr_s(buf, sizeof(buf));
	return true;
}

//bool IniSetting::GetStringFromConfig(const string& szKey, const string& szDefault, string& strString)
bool IniSetting::GetStringFromConfig(
	const string& ini_file, 
	const string section_name, 
	const string& szKey, 
	const string& szDefault, 
	string& strString)
{
    bool    fResult;
	CHAR    szValue[MAX_CONFIG_LINE]="\0";

    GetPrivateProfileStringA( section_name.c_str(),
                             szKey.c_str(),
                             "",
                             szValue,
                             MAX_CONFIG_LINE,
                             ini_file.c_str() );

    //TrimCommentAndTrailingWhitespace( szValue );

    if ( szValue[0] == '\0' )
    {
        //WriteDebug( "GetStringFromConfig returning default value for %s.\r\n",
        //            szDefault );

		//strcpy(pstrString, szDefault);
		strString = szDefault;
		fResult = true;
    }
    else
    {
        //fResult = pstrString->Copy( szValue );
		//strcpy(pstrString, szValue);
		strString = szValue;
		fResult = true;
    }

    if ( !fResult )
    {
        //WriteDebug( "GetStringFromConfig failed.  Error %d.\r\n",
        //            GetLastError() );
    }

    return fResult;
}

//int IniSetting::GetIntFromConfig(const string& szKey, int nDefault)
int IniSetting::GetIntFromConfig(
	const string& ini_file, 
	const string section_name, 
	const string& szKey, 
	int nDefault)
{
    return GetPrivateProfileIntA( section_name.c_str(),
                                 szKey.c_str(),
                                 nDefault,
                                 ini_file.c_str() );
}

IisRequestBarrierConfig::IisRequestBarrierConfig()
{
	is_base_config = false;

	switch_flag = DEFAULT_SWITCH_FLAG;
	url_match_pattern = DEFAULT_URL_MATCH_PATTERN;
	request_queue_length = DEFAULT_REQUEST_QUEUE_LENGTH;
	request_queue_duration = DEFAULT_REQUEST_QUEUE_DURATION;
	forbidden_duration = DEFAULT_FORBIDDEN_DURATION;
	forbidden_barrier = DEFAULT_FORBIDDEN_BARRIER;
}

IisRequestBarrierConfig::~IisRequestBarrierConfig()
{
}

void IisRequestBarrierConfig::ConfigBaseClass()
{
	if(!is_base_config)
	{
		ini_section_name = INI_SECTION_NAME;
		is_base_config = true;
	}
}

bool IisRequestBarrierConfig::LoadSetting()
{
	string strVal;
	char buf[MAX_CONFIG_LINE];
	int iVal = 0;

	//Set base class runtime config information before access ini file.
	ConfigBaseClass();

    if ( !GetStringFromConfig( GetIniFileName(), INI_SECTION_NAME, "URL_MATCH_PATTERN", DEFAULT_URL_MATCH_PATTERN, strVal ) )
    {
        goto Failed;
    }

	strcpy_s(buf, sizeof(buf), strVal.c_str());

	_strlwr_s(buf, sizeof(buf));

	url_match_pattern = buf;

	iVal = GetIntFromConfig( GetIniFileName(), INI_SECTION_NAME, "SWITCH_FLAG", DEFAULT_SWITCH_FLAG );

	switch_flag = (iVal != 0);

    iVal = GetIntFromConfig( GetIniFileName(), INI_SECTION_NAME, "REQUEST_QUEUE_LENGTH", DEFAULT_REQUEST_QUEUE_LENGTH );

	if(iVal<0)
	{
		//log
        goto Failed;
	}

	request_queue_length = iVal;

    iVal = GetIntFromConfig( GetIniFileName(), INI_SECTION_NAME, "REQUEST_QUEUE_DURATION", DEFAULT_REQUEST_QUEUE_DURATION );
	if(iVal<0)
	{
		//log
        goto Failed;
	}

	//convert into milliseconds
	request_queue_duration = iVal;

	iVal = GetIntFromConfig( GetIniFileName(), INI_SECTION_NAME, "FORBIDDEN_DURATION", DEFAULT_FORBIDDEN_DURATION );
	if(iVal<0)
	{
		//log
        goto Failed;
	}

	//convert into milliseconds
	forbidden_duration = iVal;

	iVal = GetIntFromConfig( GetIniFileName(), INI_SECTION_NAME, "FORBIDDEN_BARRIER", DEFAULT_FORBIDDEN_BARRIER );
	if(iVal<0)
	{
		//log
        goto Failed;
	}

	forbidden_barrier = iVal;

    return TRUE;

Failed:

    return FALSE;

}

//static
bool IisRequestBarrierConfig::isSetFileName = false;

string IisRequestBarrierConfig::ini_file_name;
string IisRequestBarrierConfig::log_file_name;

void IisRequestBarrierConfig::SetFileName()
{
	if(!isSetFileName)
	{
		isSetFileName = true;

		string strVal;

		if(GetCurrentModuleFileName(MODULE_NAME, strVal))
		{
			char buf[MAX_PATH+1];

			//strcpy(buf, strVal.c_str());
			strcpy_s(buf, sizeof(buf), strVal.c_str());
			char* dest = strstr(buf, ".dll");
			strcpy_s(dest, sizeof(buf)-(dest-buf),".ini");
			ini_file_name = buf;

			strcpy_s(dest, sizeof(buf)-(dest-buf),".log");
			log_file_name = buf;
		}
	}
}

const string& IisRequestBarrierConfig::GetIniFileName()
{
	SetFileName();

	return ini_file_name;
}

const string& IisRequestBarrierConfig::GetLogFileName()
{
	SetFileName();

	return log_file_name;
}

