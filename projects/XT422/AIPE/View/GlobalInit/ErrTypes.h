#ifndef _ERRTYPES_H_
#define _ERRTYPES_H_

enum eErrType{
	ACERR_OK=0,
	ACERR_OPENLOG=-1,		///打开LOG失败
	ACERR_CREATEFILE=-2,	///创建临时Buffer文件失败
	ACERR_PARA=-3,			///参数出现错误
	ACERR_BUFFERINIT=-4,	///Buffer初始化错误
	ACERR_UNMAPFILE=-5,		///解除Buffer映射出现错误
	ACERR_MAPFILE=-6,		///映射Buffer出现错误
	ACERR_OPENPRJ=-7,		///打开工程失败
	ACERR_PRJHEADREAD=-8,	///读取工程头部失败
	ACERR_PRJHEADMAGIC=-9,	///工程头部魔术不匹配
	ACERR_PRJHEADCHKSUM=-10,	///工程头部校验失败
	ACERR_TAGREAD=-11,			///工程标识读取失败
	ACERR_FINDTAG=-12,			///找不到指定标识
	ACERR_MEMALLOC=-13,			///分配内存空间出错
	ACERR_TAGCHKSUM=-14,		///校验标识数据出错
	ACERR_CATCHEXP=-15,			///出现异常
	ACERR_OPENDB=-16,			///打开数据库失败
	ACERR_DBSELECT=-17,			///在数据库中找不到需要的数据
	ACERR_LOADDLL=-18,			///加载校验动态库失败
	ACERR_TAGSUPPORT=-19,		///不支持的工程标识
	ACERR_SAVEFILE=-20,			///保存临时失败
	ACERR_ALLOC=-21,			///分配内存失败
	ACERR_SENDPCK=-22,			///写包数据出错
	ACERR_SENDCMD=-23,			///写命令数据出错
	ACERR_READCMD=-24,			///读命令数据出错
	ACERR_CMDEXEC=-25,			///命令执行出错
	ACERR_CMDUNSUPPORT=-26,		///机台请求的命令不支持
	ACERR_READBUF=-27,			///读取Buffer数据出错
	ACERR_WRITEBUF=-28,			///数据写入Buffer出错
	ACERR_OPENFILE=-29,			///打开文件出错
	ACERR_READFILE=-30,			///读取文件出错
	ACERR_SERSPCBUF=-31,		///设置SPCbuffer出错
	ACERR_DODEVCUSTOM=-32,		///执行驱动定制化命令失败
	ACERR_NETSEARCH=-33,		///网络扫描出错
	ACERR_INITSOCKET=-34,		///初始化Socket失败
	ACERR_STATUS=-35,			///连接状态有错误
	ACERR_CONNECT=-36,			///设备连接错误
	ACERR_DEVTYPEMATCH=-37,		///设备类型匹配错误
	ACERR_FAIL=-38,				///一般性错误
	ACERR_TOTALMATCH=-39,		///达到量产上限
	ACERR_CREATELOGDIR=-40,		///创建LOG文件夹失败
	ACERR_OCXLOAD=-41,	///加载OCX失败
	ACERR_OCXREG=-42,	///注册OCX失败
	ACERR_STOPSENSOR=-43,	///这不是真正的错误，只是告诉线程取消再次执行设置查询
	ACERR_GETSN=-44,		///从SN分配器中获取SN失败
	ACERR_MULTSNMAX=-45,	///SN已经达到上限，不能再烧录
	ACERR_CANCEL=-46,		///取消操作
	ACERR_FETCHSNSCAN=-47,  ///从SN扫描界面获取SN错误
	ACERR_FETCHSNCANCEL=-48,  ///从SN扫描界面获取SN被客户取消
	ACERR_FETCHUID=-49,  ///获取UID失败
	ACERR_NOSUPPORTUID=-50,	///不支持UID的获取

	ACERR_DBFORMAT=-100,	///数据库格式错误
	ACERR_CHKSUM=-101,		///校验值出错误

	//////////自动化设备错误信息
	ACERR_AUTOMATICCONECT=-200,			///自动化设备连接错误
	ACERR_AUTOMATICDISCONNECT=-201,		///自动化设备断开错误

	////StdMES
	ACERR_LOADMESSETTING=-301,   ////加载MES配置出现错误
	ACERR_GETDBNAME=-302,		///获取数据库文件名失败
	ACERR_MATCHCIHP=-303,		///匹配芯片名称和适配板失败
	ACERR_NOCONFIGFOUND=-304,   ///找不到Config文件
	ACERR_NOCHKFOUND=-305,		///找不到Chk文件

	///USBKey
	ACERR_CHECKCODEERR=-401,  ///USBKey校验码错误
	ACERR_QUERYINPUTCHECKCODE=-402, ///请求输入校验码错误
	ACERR_INSERTNEWQCTRL=-403,  ///插入新的控制信息错误
};

#endif
