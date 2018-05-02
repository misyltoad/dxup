#include "logger.h"
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

namespace dxup
{
	namespace Logger
	{
#ifdef _DEBUG
		class LoggerInstance
		{
		public:
			LoggerInstance()
			{
				char filename[256];

				snprintf(filename, 256, "dxup_%lu.log", (unsigned long)time(NULL));

				m_file = fopen(filename, "w");
			}

			~LoggerInstance()
			{
				if (m_file)
					fclose(m_file);
			}

			FILE* m_file;
		};

		static LoggerInstance s_Instance;

		void ManualLog(LogTypes::LogType type, const char* fmt, ...)
		{
 			if (!s_Instance.m_file)
				return;

			char buf1[1024];
			char buf2[1024];

			va_list args;
			va_start(args, fmt);
			vsnprintf(buf1, 1024, fmt, args);
			va_end(args);

			const char* typeStr = "????";
			switch (type)
			{
			case LogTypes::Fail: typeStr = "FAIL"; break;
			case LogTypes::Message: typeStr = "MESG"; break;
			case LogTypes::Warn: typeStr = "WARN"; break;
			}

			snprintf(buf2, 1024, "[%s] : %lu : %s\n", typeStr, (unsigned long)time(NULL), buf1);

			fputs(buf2, s_Instance.m_file);
			fflush(s_Instance.m_file);

			fputs(buf2, stdout);
			fflush(stdout);
		}
#endif
	}
}