#ifndef _AUTOINTERFACE_H_
#define _AUTOINTERFACE_H_

#include <string>
#include <QObject>


namespace AutomicSpace {
	class MessSingleton: public QObject {
		Q_OBJECT
	private:
		static MessSingleton* instance;

		MessSingleton() {
		}

		MessSingleton(const MessSingleton&) = delete;
		MessSingleton& operator=(const MessSingleton&) = delete;

	public:
		static MessSingleton* getInstance() {
			if (instance == nullptr) {
				instance = new MessSingleton();
			}
			return instance;
		}
	signals:
		void sendMessage(QString msg);

	};


}



/*********************
返回站点的strRdyInfo信息
{
	"ProjInfo":{
		"AdapterNum":1   ///适配板的个数，1对1还是1对8
	},
	"SiteReady":[
	{
		"SiteSN":"XXXXX",
		"SiteAlias":"XXXX",
		"SiteEnvRdy":1, ///该站点是否已经初始化好编程环境，如果为1表示已经OK，否则为0 
		"SKTRdy":"FF" ///表示站点中的SKT是否准备好，bit0-7表示SKT1到SKT8，为1表示Ready，为0表示没有
	}
	]
}
成功函数返回0，失败返回负数值
****************************/
typedef int (*FnGetDevRdyInfo)(void*Para,std::string&strRdyInfo);


typedef enum{
	COMNTYPE_NONE=0,
	COMNTYPE_UART=1,
	COMNTYPE_TCP=2,
}eAutoComnType;


typedef enum{
	AUTOPARA_BAUDRATE=1,
}eAutoParaType;

/*!
*\brief Automatic Interface
*/
class IAutomatic : public QObject{
	Q_OBJECT
signals:
	void contactICCheck(int siteIdx, QByteArray sktEn);
	void remainingICCheck(int siteIdx, QByteArray sktEn);
	void chipIsInPlace(int siteIndex, uint32_t slotEn, std::string strSiteSn);
	void programResultBack(int result, int errCode, const char* errStr);
	void printMessage(QString str);
public:
	IAutomatic()
		: m_ProtocalType(""),
		m_fnGetDevRdyInfo(nullptr),
		m_GetDevRdyInfoPara(nullptr),
		m_ProtocalVer{ 0, 0 } 
	{
		connect(AutomicSpace::MessSingleton::getInstance(), &AutomicSpace::MessSingleton::sendMessage, this, &IAutomatic::printMessage);
	}

	virtual ~IAutomatic(){};
	/*
	* \brief 初始化自动化设备
	* \param[in] ComnType : 使用那种通信方式
	* \param[in] Para	  : 通信方式附加的参数
	* \return 成功返回 0， 失败返回其他值，错误值请统一为负数
	*/
	virtual int InitAutomatic(eAutoComnType ComnType,void*Para)=0;

	/*
	* \brief 释放自动化设备
	* \return 成功返回 0，失败返回其他值，失败值请统一为负数
	*/
	virtual int ReleaseAutomatic(void)=0;


	virtual bool CheckConnect(void)=0;

	/*
	* \brief 动态设置设备的一些属性参数操作
	* \param[in] ParaType : 属性参数类型
	* \param[in] Para	  : 属性参数值
	* \return 成功返回 0， 失败返回其他值，错误值请统一为负数
	*/
	virtual int SetAutomaticPara(eAutoParaType ParaType,void*Para)=0;

	/*
	* \brief 获取自动化设备将哪些Site的IC放置好
	* \param[out] pData : bit0-bit31分别表示站点1到站点32，如果该站点IC已经放入，则置1
	*					  如果多余32个站点，则pData[1]表示33-64号站点，依次类推
	* \param[in] Size : pData有多少个单元可用
	* \return 有站点Ready则返回Ready站点的个数，如果没有则返回0,出错则返回负数的错误码
	*/
	virtual int GetICReadySite(uint32_t*pData,int Size)=0;

	
	
	/*
	* \brief 获取自动化设备将哪些Site的IC放置好
	* \param[in] SiteIdx : 站点索引从0xA开始
	* \param[in] pReady : 站点是否Ready，内存中的值为1表示Ready，为0表示未Ready
	* \param[in] pICStatus: 各个座子的放置情况，bit0-7表示SKT1-8，对应的bit为1表示该座子位置有芯片
	* \return 成功返回0,出错则返回负数的错误码
	*/
	virtual int GetICReadySite(int SiteIdx, uint8_t *pReady){return 0;};
	virtual int GetICReadySite(int SiteIdx, uint8_t *pReady, uint32_t*pICStatus){return 0;};

	virtual int ClearICReadySite( int SiteIdx)=0;


	/****************
	自动化设备中站点别名与站点索引号的映射关系,返回的模组编号从1开始
	***************/
	virtual int GetSiteIdx(std::string AutoAlias)=0; ///


	virtual void GetSiteMap(std::map<int, std::string>& vSiteMap) = 0; ///


	virtual int GetSiteIdxBySn(const std::string& sn) = 0;

	/****************
	自动化设备中站点别名与站点索引号的映射关系,返回指定的模组编号对应的站点别名
	***************/
	virtual std::string GetAutoAlias(int SiteIdx)=0;

	/*
	* \brief 告诉站点IC烧录结果
	* \param[in] SiteIdx : 站点编号1-N
	* \param[in] Result : 一个站点可能有多个Adapter同时烧录，Result指定每个Adapter的烧录情况
						  bit0-bit7表示Adapter1-Adapter8，AP8000只有Adapter1使用，Bit位为1表示成功
						  Bit位为0表示失败，AP8800最多使用Adapter1-8
						  当为AP8800时，Mask值的Bit位为1的Adapter表示使能，否则不使能，AP8000时Mask总为1
	* \param[in] Mask : 指定哪些Adapter使能
	* \return 成功返回 0，失败返回其他值，失败值请统一为负数
	*/
	virtual int SetDoneSite(int SiteIdx, uint8_t* Result, uint64_t Mask)=0;

	/*
	* \brief 请求自动化设备告知哪些站点被使能
	* \return 成功返回0，失败返回负值
	*/
	virtual int QuerySiteEn()=0;

	/*
	* \brief 告知自动化设备站点已经初始化完成，可以开始取IC进行烧录
	* \return 成功返回0，失败返回负值
	*/
	virtual int TellDevReady(std::string strRdyInfo)=0;


	/*
	* \brief 通过错误码查询错误信息
	* \param[in] ErrNo : 错误码
	* \param[out] ErrMsg : 错误码对应的消息
	*/
	virtual void GetErrMsg(int ErrNo,std::string& ErrMsg)=0;

	
	virtual bool GetProtocalVersion()=0;

	virtual int SetTask(std::string strTask)=0;


	///注册获取站点Ready信息的函数，需要返回站点是否初始化完成，已经各个站点的座子放置情况
	void SetCallBack_GetDevRdyInfo(FnGetDevRdyInfo fnGetDevRdyInfo,void*Para)
	{
		m_fnGetDevRdyInfo=fnGetDevRdyInfo;
		m_GetDevRdyInfoPara=Para;
	}
	

	///判断当前的协议版本是否大于给定的ProtocalVer版本，PrtocalVer[1]为主版本号，ProtocalVer[0]为次版本号
	bool IsCurProtocalVersionLargerThan(uint8_t ProtocalVer[2]);

	virtual int AutoTellContactICStatus(int SiteIdx, uint32_t AdpCnt, uint8_t* pResult)=0;
	virtual int AutoTellRemainingICStatus(int SiteIdx, uint32_t AdpCnt, uint8_t* pResult)=0;
	
public:
	std::string m_ProtocalType;
	//CConfigSetting m_ConfigSetting;

protected:
	FnGetDevRdyInfo m_fnGetDevRdyInfo;
	void* m_GetDevRdyInfoPara;
	uint8_t m_ProtocalVer[2]; ///协议的版本号 [1]为Major [0]为Minor，比如0x01 0x00表示1.0
};

/************************************************************************/
/* \brief 获取自动化设备接口                                            
/* \param[in] Manufacture : 自动化设备厂商
/* \param[in] MType ：		自动化设备型号
/* \return 自动化设备接口
/* 需要调用PutAutomatic进行资源释放
/************************************************************************/
IAutomatic* GetAutomatic(std::string ProtocalType, std::string& errStr);


bool CheckAutomaticConnect(IAutomatic*pIFace);

/************************************************************************/
/* 释放自动化设备的资源 
/* 成功返回0 ，失败返回负数
/************************************************************************/
int PutAutomatic(IAutomatic* pIFace);

/************************************************************************/
/* 失败返回-1，检查到IC放下返回1，未检查到IC返回0                       */
/************************************************************************/
int CheckAutomaticSiteICReady(IAutomatic *pIFace,std::string& AutoAlias,uint8_t*pAutoChangeAdp,uint8_t*pICStatus);

int ClearAutomaticSiteICReady(IAutomatic *pIFace,std::string& AutoAlias);

/************************************************************************/
/* 设置站点执行结果，AutoAlias为自动化站点别名，
/* Result : 一个站点可能有多个Adapter同时烧录，Result指定每个Adapter的烧录情况
bit0-bit7表示Adapter1-Adapter8，AP8000只有Adapter1使用，Bit位为1表示成功
Bit位为0表示失败，AP8800最多使用Adapter1-8
当为AP8800时，Mask值的Bit位为1的Adapter表示使能，否则不使能，AP8000时Mask总为1
/* Mask : 指定哪些Adapter使能
/* 成功返回 0 失败返回负值 
/************************************************************************/
int SetAutomaticSiteDone(IAutomatic *pIFace,std::string& AutoAlias, uint8_t* Result,uint32_t Mask);
#endif 