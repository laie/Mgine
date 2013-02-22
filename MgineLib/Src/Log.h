#pragma once

#pragma region Macro
	/*\  Message Log
	Not deleted on release mode build
	\*/
	#define MLOG(Message, ...) \
		( \
		Util::Log::WriteLog(L"[%s{%d}] " L ## Message, \
			__FUNCTIONW__, \
			__LINE__, \
			__VA_ARGS__ \
			), \
			false \
		)
#pragma endregion



namespace Util
{
	class Log
	{
	public:
		class clsInitialLog;

	private:
	public:
		static	clsInitialLog InitiailLog;

		static	wchar_t FileNameLog[MAX_PATH];

		static 	HANDLE hFile;

		static	bool Start();
		static 	bool ClearLog();
		static 	bool WriteLine(const wchar_t *LogStr);
		static	bool WriteLog(const wchar_t *LogStr, ...);
		static	bool Write(const wchar_t *LogStr);
		static	bool Flush();
		static	bool Finish();
		static	wchar_t *GetLogFileName();



		class clsInitialLog
		{
		public:
			clsInitialLog();
			~clsInitialLog();
		};
	};
}
