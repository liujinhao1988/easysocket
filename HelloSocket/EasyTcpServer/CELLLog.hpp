//打印日志，将 输出日志写入txt中
//单例模式




#ifndef _CELL_LOG_HPP_
#define _CELL_LOG_HPP_

#include "CELL.hpp"
#include"CELLTask.hpp"
#include<ctime>
class CELLTaskServer;
class CELLLog
{
	//Info
	//Debug
	//warnning
	//Error
private:
	CELLLog()
	{
		_taskServer.Start();
	}
	~CELLLog()
	{
		_taskServer.Close();
		if (_logFile)
		{
			Info("CELLLog fclose(_logFile)\n");
			fclose(_logFile);
			_logFile = nullptr;
		}
	}
public:
	static CELLLog& Instance()
	{
		static CELLLog sLog;
		return sLog;
	}

	void setLogPath(const char* logPath, const char* mode)
	{
		if (_logFile)
		{
			Info("CELLLog::setLogPath _logFile != nullptr\n");
			fclose(_logFile);//关闭文件
			_logFile = nullptr;
		}
		
		_logFile = fopen(logPath, mode);
		if (_logFile)
		{
			Info("CELLLog::setLogPath success,<%s,%s>\n", logPath, mode);
		}
		else
		{
			Info("CELLLog::setLogPath failed,<%s,%s>\n", logPath, mode);
		}
	}
	
	static void Info(const char* pStr)
	{
		//pLog, pformat, args...
		CELLLog* pLog = &Instance();
		pLog->_taskServer.addTask([=]() {
			if (pLog->_logFile)
			{
				auto t = system_clock::now();
				auto tNow = system_clock::to_time_t(t);
				//写入时间
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));
				std::tm* now = std::gmtime(&tNow);
				fprintf(pLog->_logFile, "%s", "Info");
				//写入文件
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);//写入时间
																																						   //写入文件
				fprintf(pLog->_logFile, "%s", pStr);
				fflush(pLog->_logFile);//实时写入
			}
			printf( "%s", pStr);

		});

		
	}

	template<typename ... Args>//可变参数
	static void Info(const char* pformat, Args ... args)
	{
		//pLog, pformat, args...
		CELLLog* pLog = &Instance();
		pLog->_taskServer.addTask([=]() {
			if (pLog->_logFile)
			{
				auto t = system_clock::now();
				auto tNow = system_clock::to_time_t(t);
				//写入时间
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));
				std::tm* now = std::gmtime(&tNow);
				fprintf(pLog->_logFile, "%s", "Info");
				//写入文件
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);//写入时间
																																						   //写入文件
				fprintf(pLog->_logFile, pformat, args...);
				fflush(pLog->_logFile);//实时写入
			}
			printf(pformat, args...);
		
		});
		
	}
private:

	FILE* _logFile = nullptr;
	CELLTaskServer _taskServer;
};



#endif // !_CELL_LOG_HPP_
