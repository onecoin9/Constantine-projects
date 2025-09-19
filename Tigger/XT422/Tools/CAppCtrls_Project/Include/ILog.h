#ifndef _ILOG_H_
#define _ILOG_H_

#define LOGLEVEL_D		(0)		///调试日志
#define LOGLEVEL_N		(1)		///一般日志
#define LOGLEVEL_W		(2)		///警告日志
#define LOGLEVEL_E		(3)     ///错误日志


class ILog{
public:
	ILog():m_bTimeHeadEn(true)
		,m_LogLevel(LOGLEVEL_N)
	{

	}
	/// <summary>
	/// 设置日志输出登记，只有大于等于这个日志等级的日子才会被输出
	/// </summary>
	/// <param name="LogLevel"></param>
	void SetLogLevel(int LogLevel) {
		m_LogLevel = LogLevel;
	}
	virtual void PrintLog(int32_t Level,const char*fmt,...)=0;
	virtual void PrintBuf(char* pHeader, char* pData, int Size) = 0;
	virtual void TimeHeadEn(bool En) {
		m_bTimeHeadEn = En;
	};

protected:
	bool m_bTimeHeadEn;
	int m_LogLevel;
};

#endif 