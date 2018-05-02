#pragma once

namespace dxup
{
	namespace Logger
	{
		namespace LogTypes
		{
			enum LogType
			{
				Fail,
				Warn,
				Message
			};
		}


#ifdef _DEBUG
		void ManualLog(LogTypes::LogType type, const char* fmt, ...);
#define DXUP_Log(type, fmt, ...) ::dxup::Logger::ManualLog(::dxup::Logger::LogTypes::##type , "%s : Line %d ~->| " fmt, __FILE__, __LINE__, __VA_ARGS__)
#define DXUP_Assert(condition) if (!(condition)) { DXUP_Log(Fail, "Assertion failed: %s", #condition ); }
#define DXUP_AssertSuccess(hresult) DXUP_Assert( (!FAILED(hresult)) )
#else
#define DXUP_Log(type, fmt, ...)
#define DXUP_Assert(condition)
#define DXUP_AssertSuccess(hresult)
#endif
	}
}