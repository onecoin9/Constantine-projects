#ifndef _ILOG_H_
#define _ILOG_H_

#define LOGLEVEL_D		(0)		///������־
#define LOGLEVEL_N		(1)		///һ����־
#define LOGLEVEL_W		(2)		///������־
#define LOGLEVEL_E		(3)     ///������־


class ILog{
public:
	ILog():m_bTimeHeadEn(true)
		,m_LogLevel(LOGLEVEL_N)
	{

	}
	/// <summary>
	/// ������־����Ǽǣ�ֻ�д��ڵ��������־�ȼ������ӲŻᱻ���
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