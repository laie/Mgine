#pragma once
namespace Mgine
{
	class LocalSettingManager
	{
	private:
		static wchar_t SettingKeyName[1024];
		static HKEY hKey;

	public:
		struct SettingChunk
		{
			LOCALSETTING_TYPE Type;
			wchar_t KeyPath[1024];
			wchar_t KeyName[1024];
			void *Pointer;
			DWORD Size;
		};

		static bool IsInited;
		static void Init(wchar_t *SettingKeyName);
		static void Uninit();

		static void SetLocalSetting(wchar_t *SubKey, wchar_t *LocalSettingIdentifier, LOCALSETTING_TYPE ValueType, void *Value, DWORD ValueSize);
		static void GetLocalSetting(wchar_t *SubKey, wchar_t *LocalSettingIdentifier, LOCALSETTING_TYPE ValueType, void *Value, DWORD *ValueSize);
	};
}