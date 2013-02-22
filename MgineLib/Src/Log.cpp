#include "StdAfx.hpp"
#include <intrin.h>
#include <time.h>

using namespace Mgine;
using namespace Util;

Log::clsInitialLog InitiailLog;
HANDLE		Log::hFile;
wchar_t		Log::FileNameLog[MAX_PATH];

bool Log::Start()
{
	//asctime(
	ClearLog();
	hFile = CreateFile(GetLogFileName(),
		GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if ( !hFile || hFile == INVALID_HANDLE_VALUE ) return false;

	wchar_t procstr[4096];
	WriteLine(L":: Log Started ::");

	int cpuid[4];
	__cpuid(cpuid, 0x80000000);
	DWORD nexId = cpuid[0];
	char cpubrand[0x40] = { 0, };
	wchar_t wcpubrand[0x40];

	for ( DWORD i=0x80000000; i <= nexId; i++ )
	{
		__cpuid(cpuid, i);
		if ( 0x80000002 <= i && i <= 0x80000004 )
		{
			memcpy(cpubrand+(i-0x80000002)*sizeof(cpuid), cpuid, sizeof(cpuid));
		}
	}

	MultiByteToWideChar(CP_ACP, 0, cpubrand, 0x40, wcpubrand, 0x40);

	WriteLine(L"");
	WriteLine(L"\t:: Spec printing.. ::");
	swprintf_s(procstr, L"\tProcessor\t%s", wcpubrand);
	WriteLine(procstr);

	ULONGLONG memsize = -1;
	if ( !GetPhysicallyInstalledSystemMemory(&memsize) )
	{
		WriteLine(L"\tFailed to get ram info");
	} else {
		swprintf_s(procstr, L"\tRam Size\t%lluMB", memsize/1024);
		WriteLine(procstr);
	}

	IDirect3D9 *pd3d = Direct3DCreate9(D3D9b_SDK_VERSION);
	DWORD countadapter = pd3d->GetAdapterCount();
	for ( DWORD i=0; i < countadapter; i++ )
	{
		D3DADAPTER_IDENTIFIER9 adapter;
		wchar_t adapterdesc[MAX_DEVICE_IDENTIFIER_STRING] = { 0, };

		memset(&adapter, 0, sizeof(D3DADAPTER_IDENTIFIER9));

		pd3d->GetAdapterIdentifier(i, 0, &adapter);

		MultiByteToWideChar(CP_ACP, 0,
			adapter.Description, MAX_DEVICE_IDENTIFIER_STRING,
			adapterdesc, MAX_DEVICE_IDENTIFIER_STRING);


		swprintf_s(procstr, L"\tAdapter %d\t%s", i+1, adapterdesc);
		WriteLine(procstr);
	}
	pd3d->Release();

	WriteLine(L"\t:: Done ::");
	WriteLine(L"");

	return true;
}

bool Log::ClearLog()
{
	//LPWCH w = GetEnvironmentStringsW();
	
	return DeleteFile(GetLogFileName()) != 0;
}

bool Log::WriteLine(const wchar_t *LogStr)
{
	if ( !Write((std::wstring(LogStr) + L"\r\n").c_str())  ) return false;
	return true;
}

bool Log::WriteLog(const wchar_t *LogStr, ...)
{
	va_list vl;
	va_start(vl, LogStr);

	SYSTEMTIME time;
	GetSystemTime(&time);

	DWORD lenprocstr = wcslen(LogStr)+1024;
	UnwindArray<wchar_t> procstr = new wchar_t[lenprocstr];
	UnwindArray<wchar_t> resstr  = new wchar_t[lenprocstr];

	swprintf_s(procstr.Arr, lenprocstr, L"[%.04d-%.02d-%.02d %.02d:%.02d:%.02d]\t%s",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, LogStr);
	vswprintf_s(resstr.Arr, lenprocstr, procstr.Arr, vl);
	bool isSucceeded = WriteLine(resstr.Arr);

	//delete resstr;
	//delete procstr;


	va_end(vl);
	return isSucceeded;
}

bool Log::Write(const wchar_t *LogStr)
{
	DWORD written;
#ifdef _DEBUG
	OutputDebugStringW(LogStr);
#endif
	return WriteFile(hFile, LogStr, wcslen(LogStr)*2, &written, NULL) != 0;
}

bool Log::Flush()
{
	return FlushFileBuffers(hFile) != 0;
}

bool Log::Finish()
{
	bool isSucceeded = true;
	if ( !Log::WriteLine(L":: Log Finished ::") ) isSucceeded = false;
	if ( !Log::Flush() ) isSucceeded = false;
	CloseHandle(hFile);
	hFile = 0;
	return isSucceeded;
}

wchar_t *Log::GetLogFileName()
{
	wchar_t username[1024];
	DWORD sizeuserName;
	GetUserName(username, &sizeuserName);
	swprintf_s(FileNameLog, MAX_PATH, L"Mgine_%s.log", username);
	return FileNameLog;
}








Log::clsInitialLog::clsInitialLog()
{
	Log::Start();
}

Log::clsInitialLog::~clsInitialLog()
{
	Log::Finish();
}

