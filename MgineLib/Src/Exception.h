#pragma once

/*#include <stdexcept>

namespace Mgine
{
	enum EXCEPTION_TYPE // this is just for intellisense convenience
	{
		General = 0,
		NotImplemented,
		Unsupported,
		DirectXError,
		Win32Error,
		InvalidStatus,
		InvalidOperation,
		InvalidParameter,
		OutOfMemory,
		OutOfRange,
		ValueOverflow,
		ElementNotExisting,
		AccessDenied,
		NotFound,
		Assertion,
		NeverReach,
		OperationCanceled,
		EXCEPTION_TYPE_COUNT,
	};

	class BaseException : public virtual ::std::exception
	{
	protected:
		std::wstring	Message;
		std::string		MessageMB;

		inline void InitVAMsg(const std::wstring ExceptionName, const std::wstring & ExceptionMessage, va_list Args)
		{
			int lenbuf = ExceptionMessage.size()+4096;
			UnwindArray<wchar_t> procstr		= new wchar_t[lenbuf];
			UnwindArray<wchar_t> procstrreflect	= new wchar_t[lenbuf];
			wcscpy_s(procstrreflect, lenbuf, ExceptionMessage.c_str());

			swprintf_s(procstr.Arr, lenbuf, L"[Exception: %s] %s", ExceptionName.c_str(), procstrreflect);
			vswprintf_s(procstrreflect.Arr, lenbuf, procstr.Arr, Args);
			Message = std::wstring(procstrreflect.Arr);
			MessageMB = strconv(Message);

			__if_exists(Log)
			{
				Log::WriteLog(L"%s", Message.c_str());
			}
		}

		inline BaseException(const std::wstring & ExceptionMessage) : Message(ExceptionMessage) { }
		inline BaseException(){ }


	public:
		virtual const char *what() const
		{ return MessageMB.c_str(); }
		virtual const wchar_t *GetMessage() const
		{ return Message.c_str(); }

	};

	#define DECLARE_EXCEPTION(ExceptionType) \
		class ExceptionType##Exception : public virtual BaseException \
		{ \
		public: \
			inline ExceptionType##Exception(const wchar_t *ExceptionMessage, ...) \
			{ \
				va_list vl; \
				va_start(vl, ExceptionMessage); \
				InitVAMsg(L## #ExceptionType, ExceptionMessage, vl); \
				va_end(vl); \
			} \
		}
	DECLARE_EXCEPTION(General);
	DECLARE_EXCEPTION(NotImplemented);
	DECLARE_EXCEPTION(Unsupported);
	DECLARE_EXCEPTION(DirectXError);
	DECLARE_EXCEPTION(Win32Error);
	DECLARE_EXCEPTION(InvalidStatus);
	DECLARE_EXCEPTION(InvalidOperation);
	DECLARE_EXCEPTION(InvalidParameter);
	DECLARE_EXCEPTION(OutOfMemory);
	DECLARE_EXCEPTION(OutOfRange);
	DECLARE_EXCEPTION(ValueOverflow);
	DECLARE_EXCEPTION(ElementNotExisting);
	DECLARE_EXCEPTION(AccessDenied);
	DECLARE_EXCEPTION(NotFound);
	DECLARE_EXCEPTION(Assertion);
	DECLARE_EXCEPTION(NeverReach);
	DECLARE_EXCEPTION(OperationCanceled);

}



*/