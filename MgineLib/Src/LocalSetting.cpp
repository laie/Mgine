#include "StdAfx.hpp"
#include "LocalSetting.h"
using namespace Mgine;
using namespace Util;

wchar_t LocalSettingManager::SettingKeyName[1024];
HKEY	LocalSettingManager::hKey;
bool	LocalSettingManager::IsInited;


void LocalSettingManager::Init(wchar_t *SettingKeyName)
{
	if ( IsInited ) MTHROW(InvalidStatus, "Already Inited");
	MLOG("Begin");
	try
	{ 
		memset(LocalSettingManager::SettingKeyName, 0, sizeof(LocalSettingManager::SettingKeyName));
		wcscpy_s(LocalSettingManager::SettingKeyName, SettingKeyName);

		wchar_t procstr[1024];
		swprintf_s(procstr, 1024, L"Software\\%s", LocalSettingManager::SettingKeyName);
		if ( RegCreateKeyEx(HKEY_CURRENT_USER, procstr,
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			NULL,
			&hKey,
			NULL)
			) MTHROW(Win32Error, "RCKE");

		IsInited = true;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		Uninit();
		throw;
	}
}

void LocalSettingManager::Uninit()
{
	IsInited = false;
	RegFlushKey(hKey);
	CloseHandle(hKey);
	hKey = NULL;
}

void LocalSettingManager::SetLocalSetting(wchar_t *SubKey, wchar_t *LocalSettingIdentifier, LOCALSETTING_TYPE ValueType, void *Value, DWORD ValueSize)
{
	if ( !hKey ) MTHROW(InvalidOperation, "hKey is null");
	HKEY hk = NULL;

	try
	{ 
		if ( RegCreateKeyEx(hKey, SubKey, 
			REG_OPTION_NON_VOLATILE, 
			NULL, 
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
			&hk, NULL) ) MTHROW(Win32Error, "RCKE");

		DWORD dwtype = NULL;

		switch ( ValueType )
		{
		case LOCALSETTING_BINARY:
			dwtype = REG_BINARY;
			break;

		case LOCALSETTING_DWORD:
			dwtype = REG_DWORD;
			ValueSize = 4;
			break;

		case LOCALSETTING_QWORD:
			dwtype = REG_QWORD;
			ValueSize = 8;
			break;

		case LOCALSETTING_STRING:
			dwtype = REG_SZ;
			if ( ValueSize == -1 )
			{
				ValueSize = wcslen((wchar_t*)Value); 
			} else {
				ValueSize = wcsnlen_s((wchar_t*)Value, ValueSize); 
			}
			ValueSize *= 2;
			break;

		default:
			MTHROW(InvalidParameter, "Unknown Type");
		}
		if ( RegSetValueEx(hk, LocalSettingIdentifier, 0, dwtype, (BYTE*)Value, ValueSize)
			) MTHROW(Win32Error, "RSVE");

		CloseHandle(hk);
		hk = NULL;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		CloseHandle(hk);
		hk = NULL;
		throw;
	}
}

void LocalSettingManager::GetLocalSetting(wchar_t *SubKey, wchar_t *LocalSettingIdentifier, LOCALSETTING_TYPE ValueType, void *Value, DWORD *ValueSize)
{
	if ( !hKey ) MTHROW(InvalidOperation, "hKey is null");
	if ( !Value ) MTHROW(InvalidParameter, "Value is null");

	HKEY hk;
	try
	{ 
		if ( RegCreateKeyEx(hKey, SubKey, 
			REG_OPTION_NON_VOLATILE,
			NULL,
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
			&hk, NULL)
		) MTHROW(Win32Error, "RCKE");

		DWORD dwrestrict = NULL;
		DWORD type = 0;
		DWORD localsize = 0;

		switch ( ValueType )
		{
			case LOCALSETTING_BINARY:
				if ( !ValueSize ) MTHROW(InvalidParameter, "VS is null");
				dwrestrict = RRF_RT_REG_BINARY;
				break;

			case LOCALSETTING_DWORD:
				if ( !ValueSize )
				{
					ValueSize = &localsize;
					localsize = 4;
				} else *ValueSize = 4;
				dwrestrict = RRF_RT_DWORD;
				break;

			case LOCALSETTING_QWORD:
				if ( !ValueSize )
				{
					ValueSize = &localsize;
					localsize = 8;
				} else *ValueSize = 8;
				dwrestrict = RRF_RT_QWORD;
				break;

			case LOCALSETTING_STRING:
				if ( !ValueSize ) MTHROW(InvalidParameter, "VS is null");
				dwrestrict = RRF_RT_REG_SZ;
				*ValueSize *= 2;
				break;

			default:
				MTHROW(InvalidOperation, "Unknown Type");
		}
		/*if ( RegGetValue(hk, NULL, LocalSettingIdentifier, 
			dwrestrict | RRF_ZEROONFAILURE, &type, Value, ValueSize)
		) goto cleanup;*/

		if ( RegQueryValueEx(hk, LocalSettingIdentifier, NULL, &type, (BYTE*)Value, ValueSize) )
			MTHROW(Win32Error, "RQVE. GLE: %d", __FUNCTIONW__, __LINE__);

		if ( ValueType == LOCALSETTING_STRING )
			if ( ValueSize ) *ValueSize /= 2;

		CloseHandle(hk);
		hk = NULL;
	}
	catch ( BaseException & )
	{
		MLOG("Failed");
		CloseHandle(hk);
		hk = NULL;
		if ( ValueType == LOCALSETTING_STRING )
			if ( ValueSize ) *ValueSize /= 2;
		throw;
	}
}

