#include "ComTool.h"
#include "DataBufferDefine.h"
#include <qvector.h>
#include <QTextCodec>
#include <QRegularExpression>
/**********************************************
@brief 将给定的字符串按照10进制进行转换，转换结果为16进制，例如字符串为255,则转换结果过为0xFF
@param[in]	str			需要转换的字符串
@param[in]	EndianType	结果数据存放格式
@param[out] vDecDataOut	转换结果存放在vDecData中,EndianType=ENDIAN_BIG第一个字节为最高字节,否则第一个字节为最低字节
@return
	true	转换成功
	false	包含非法字符，转换失败
************************************************/
bool ComTool::Str2Dec(QString&str,unsigned int EndianType,QVector<unsigned char>&vDecDataOut)
{
	vDecDataOut.clear();
	unsigned char AddInc=0,cData;
	unsigned short Tmp=0;
	int i,len,j;
	QVector<unsigned char>vDecData;
	len=str.length();
	vDecData.push_back(AddInc);
	for(i=0;i<len;i++){
		cData=str[i].cell();
		if(!isdigit(cData)){
			return false;
		}
		for(j=0;j<(int)vDecData.size();j++){
			if(j==0){
				Tmp=vDecData[j]*10+(cData-'0');
			}
			else{
				Tmp=vDecData[j]*10+AddInc;
			}
			AddInc=(Tmp>>8)&0xFF;
			vDecData[j]=(unsigned char)Tmp;///写回去
		}
		if(AddInc!=0){//有进位需要再次产生一个
			vDecData.push_back(AddInc);
		}	
	}
	if(EndianType&ENDIAN_BIG){
		for(i=(int)vDecData.size()-1;i>=0;i--){
			vDecDataOut.push_back(vDecData[i]);
		}
	}
	else{
		vDecDataOut=vDecData;
	}
	return true;
}

/**********************************************
@brief 将给定的数据按照10进制进行转换输出
@param[in] EndianType	指定vDec的数据组织方式
@param[in] vDec			需要转换的数据，EndianType=ENDIAN_BIG时高字节在第一个字节，否则低字节在第一个字节
@param[out] strData		输出的字符串
@return
true	转换成功
false	包含非法字符，转换失败
************************************************/
bool ComTool::Dec2Str(unsigned int EndianType,QVector<unsigned char>&vDec,QString& strData)
{
	int Size=(int)vDec.size();
	unsigned char *pTmp=new unsigned char[Size];
	unsigned char Left=0;
	unsigned short Tmp=0;
	int i,ZeroCnt=0;
	if(!pTmp){
		return false;
	}
	strData="";
	if(EndianType&ENDIAN_BIG){
		for(i=0;i<Size;i++){
			pTmp[i]=vDec[i];
		}
	}
	else{
		for(i=0;i<Size;i++){///转成大端处理方式
			pTmp[i]=vDec[Size-1-i];
		}
	}
	////统一用大端处理
	while(ZeroCnt!=Size){///全部为0表明转换完成
		ZeroCnt=0;
		Tmp=0;
		Left=0;
		for(i=0;i<Size;i++){
			Tmp+=pTmp[i];
			Left=Tmp%10;
			pTmp[i]=Tmp/10;
			Tmp=Left<<8;///将余数放到高8位
		}
		strData.insert(-1,Left+'0');
		for(i=0;i<Size;i++){
			if(pTmp[i]==0){///全部是0则退出
				ZeroCnt++;
			}
		}
	}

	if(pTmp){
		delete[] pTmp;
	}
	return true;
}

/**********************************************
@brief 将给定的数据按照10进制进行转换输出
@param[in] EndianType	指定pData的数据组织方式
@param[in] pData	需要转换的数据，EndianType=ENDIAN_BIG时高字节在第一个字节，否则低字节在第一个字节
@param[in] Size		转转的数据占几个字节
@param[out] strData	输出的字符串
@return
	true	转换成功
	false	包含非法字符，转换失败
************************************************/
bool ComTool::Dec2Str(unsigned int EndianType,unsigned char*pData,int Size,QString& strData)
{
	int i;
	QVector<unsigned char>vDec;
	for(i=0;i<Size;i++){
		vDec.push_back(pData[i]);
	}
	return Dec2Str(EndianType,vDec,strData);
}

/**********************************************
@brief 将给定的字符串按照16进制进行转换，转换结果为16进制，例如字符串为255,则转换结果过为0x255
@param[in]	str			需要转换的字符串
@param[in]	EndianType	结果数据存放格式
@param[out] vDecDataOut	转换结果存放在vDecData中,EndianType=ENDIAN_BIG第一个字节为最高字节,否则第一个字节为最低字节
@return
	true	转换成功
	false	包含非法字符，转换失败
************************************************/
bool ComTool::Str2Hex( QString&str,unsigned int EndianType,QVector<unsigned char>&vDecDataOut)
{
	vDecDataOut.clear();
	unsigned char AddInc=0,cData;
	unsigned char ByteData;
	unsigned short Tmp=0;
	int i,len,j;
	QVector<unsigned char>vDecData;
	len=str.length();
	if((EndianType&PREHEADZERO_NEED)==0){
		vDecData.push_back(AddInc);
		for(i=0;i<len;i++){
			cData=str[i].cell();
			if(isdigit(cData)){
				cData=cData-'0';
			}
			else if(cData>='A'&&cData<='F'){
				cData=cData-'A'+10;
			}
			else if(cData>='a'&&cData<='f'){
				cData=cData-'a'+10;
			}
			else{
				return false;
			}
			for(j=0;j<(int)vDecData.size();j++){
				if(j==0){
					Tmp=vDecData[j]*16+cData;
				}
				else{
					Tmp=vDecData[j]*16+AddInc;
				}
				AddInc=(Tmp>>8)&0xFF;
				vDecData[j]=(unsigned char)Tmp;///写回去
			}
			if(AddInc!=0){//有进位需要再次产生一个
				vDecData.push_back(AddInc);
			}	
		}
	}
	else{///不能忽略前导0
		if(len%2!=0){///需要是2的整数倍
			return false;
		}
		for(i=len-1;i>=0;i--){
			cData=str[i].cell();
			if(isdigit(cData)){
				cData=cData-'0';
			}
			else if(cData>='A'&&cData<='F'){
				cData=cData-'A'+10;
			}
			else if(cData>='a'&&cData<='f'){
				cData=cData-'a'+10;
			}
			else{
				return false;
			}
			if(i%2==0){
				ByteData +=cData<<4;
				vDecData.push_back(ByteData);
			}
			else{
				ByteData=cData;
				
			}
		}
	}
	if(EndianType&ENDIAN_BIG){
		for(i=(int)vDecData.size()-1;i>=0;i--){
			vDecDataOut.push_back(vDecData[i]);
		}
	}
	else{
		vDecDataOut=vDecData;
	}
	return true;
}

/**********************************************
@brief 将给定的数据按照16进制进行转换输出
@param[in] EndianType	决定vHex的数据组织方式 为ENDIAN_BIG时高字节在第一个字节，否则低字节在第一个字节
@param[in] vHex			需要转换的数据
@param[out] strData		输出的字符串
@return
true	转换成功
false	包含非法字符，转换失败
************************************************/
bool ComTool::Hex2Str(unsigned int EndianType,QVector<unsigned char>&vHex,QString& strData)
{
	int Size=(int)vHex.size();
	unsigned char *pTmp=new unsigned char[Size];
	unsigned char Left=0;
	unsigned short Tmp=0;
	int i,ZeroCnt=0;
	if(!pTmp){
		return false;
	}
	strData="";
	if(EndianType&ENDIAN_BIG){
		for(i=0;i<Size;i++){
			pTmp[i]=vHex[i];
		}
	}
	else{
		for(i=0;i<Size;i++){///转成大端处理方式
			pTmp[i]=vHex[Size-1-i];
		}
	}
	///统一用大端处理
	while(ZeroCnt!=Size){///全部为0表明转换完成
		ZeroCnt=0;
		Tmp=0;
		Left=0;
		for(i=0;i<Size;i++){
			Tmp+=pTmp[i];
			Left=Tmp%16;
			pTmp[i]=Tmp/16;
			Tmp=Left<<8;///将余数放到高8位
		}
		if(Left<10){
			strData.insert(-1,Left+'0');
		}
		else{
			strData.insert(-1,Left-10+'A');
		}
		for(i=0;i<Size;i++){
			if(pTmp[i]==0){///全部是0则退出
				ZeroCnt++;
			}
		}
	}

	if(pTmp){
		delete[] pTmp;
	}
	return true;
}

/**********************************************
@brief 将给定的数据按照16进制进行转换输出
@param[in] pData	需要转换的数据，高字节在第一个字节
@param[in] Size		转转的数据占几个字节
@param[out] strData	输出的字符串
@return
true	转换成功
false	包含非法字符，转换失败
************************************************/
bool ComTool::Hex2Str(unsigned int EndianType,unsigned char*pData,int Size,QString& strData)
{
	int i;
	QVector<unsigned char>vHex;
	for(i=0;i<Size;i++){
		vHex.push_back(pData[i]);
	}
	return Hex2Str(EndianType,vHex,strData);
}

void LSBShiftNBytes(unsigned char *pData,int Size,int nBytes)
{
	int i=0,ByteLeft=Size-nBytes;
	if(nBytes==0){
		return;
	}
	for(i=0;i<ByteLeft;i++){
		pData[Size-1-i]=pData[Size-1-nBytes-i];
	}
	for(i=0;i<nBytes;i++){///将低位清0
		pData[i]=0;
	}
}

/**********************************************
@brief 将给定的两个16进制数据相乘
@param[in] EndianType 决定vMul1，vMul2，vDecDataOut的数据组织方式
@param[in] vMul1	乘数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
@param[in] vMul2	被乘数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
@param[out] vDecDataOut	输出的乘积，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
@return
true	成功
false	失败
************************************************/
bool ComTool::MultiBytesMul(unsigned int EndianType,QVector<unsigned char>&vMul1,QVector<unsigned char>&vMul2,QVector<unsigned char>&vDecDataOut)
{
	int i,j,MulCnt,h;
	bool Ret=true;
	unsigned char AddInc=0,Left=0;
	unsigned int Mul=0;
	int Size1,Size2;
	QVector<unsigned char>vDecData;
	unsigned char*pResult=NULL,*pResult1=NULL,*pResult2=NULL;
	unsigned char *pTmpMul=NULL,*pTmpMul1=NULL,*pTmpMul2=NULL;
	Size1=(int)vMul1.size();
	Size2=(int)vMul2.size();
	MulCnt=Size1+Size2;
	pTmpMul=new unsigned char[Size1+Size2];
	pResult=new unsigned char[MulCnt*2];///分配两倍的Size1+Size2用于存放临时结果,乘积最大占Size1+Size2个字节
	if(pTmpMul==NULL || pResult==NULL){
		Ret=false;
		goto __end;
	}
	///将数据转成小端存放
	memset(pTmpMul,0,Size1+Size2);
	memset(pResult,0,MulCnt*2);
	pTmpMul1=pTmpMul;
	pTmpMul2=pTmpMul+Size1;
	pResult1=pResult;
	pResult2=pResult+MulCnt;
	if(EndianType&ENDIAN_BIG){
		for(i=0;i<Size1;i++){
			pTmpMul1[Size1-1-i]=vMul1[i];
		}
		for(i=0;i<Size2;i++){
			pTmpMul2[Size2-1-i]=vMul2[i];
		}
	}
	else{
		for(i=0;i<Size1;i++){
			pTmpMul1[i]=vMul1[i];
		}
		for(i=0;i<Size2;i++){
			pTmpMul2[i]=vMul2[i];
		}
	}
	////统一用小端处理
	unsigned char Ch1,Ch2;
	for(j=0;j<Size2;j++){
		Ch2=pTmpMul2[j];
		AddInc=0;
		for(i=0;i<Size1;i++){///多字节乘以1字节
			Ch1=pTmpMul1[i];
			Mul=Ch1*Ch2+AddInc;
			AddInc=(Mul>>8)&0xFF;
			Left=Mul&0xFF;
			pResult2[i]=Left;
		}
		if(AddInc){
			pResult2[i]=AddInc;
		}
		LSBShiftNBytes(pResult2,MulCnt,j);///根据乘数字节的位置进行乘积字节偏移
		MultiBytesAdd(ENDIAN_LIT,pResult1,MulCnt,pResult2,MulCnt,vDecData);
		memset(pResult1,0,MulCnt);
		memset(pResult2,0,MulCnt);
		for(h=0;h<(int)vDecData.size();h++){///为了下次的计算将结果保存到pResult1中，
			pResult1[h]=vDecData[h];
		}
	}

	for(i=MulCnt-1;i>=0;i--){///去掉前面的0
		if(pResult1[i]!=0){
			break;
		}
	}

	if(i<0){///乘积为0,则只产生一个字节的0
		vDecDataOut.push_back(pResult1[0]); 
	}

	if(EndianType&ENDIAN_BIG){
		for(;i>=0;i--){///开始存放结果
			vDecDataOut.push_back(pResult1[i]);
		}
	}
	else{
		int ByteCnt=i;
		for(i=0;i<=ByteCnt;i++){///开始存放结果
			vDecDataOut.push_back(pResult1[i]);
		}
	}

__end:
	if(pTmpMul)
		delete pTmpMul;
	if(pResult)
		delete pResult;
	return Ret;
}

/**********************************************
@brief 将给定的两个16进制数据相乘
@param[in] EndianType 决定pMul1，pMul2，vDecDataOut的数据组织方式
@param[in] pMul1	乘数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
@param[in] Size1	乘数占几个字节
@param[in] pMul2	被乘数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
@param[in] Size2	被乘数占几个字节
@param[out] vDecDataOut	输出的乘积，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
@return
true	成功
false	失败
************************************************/
bool ComTool::MultiBytesMul(unsigned int EndianType,unsigned char*pMul1,int Size1,unsigned char*pMul2,int Size2,QVector<unsigned char>&vDecDataOut)
{
	int i;
	QVector<unsigned char>vMul1;
	QVector<unsigned char>vMul2;
	for(i=0;i<Size1;i++)
		vMul1.push_back(pMul1[i]);
	for(i=0;i<Size2;i++)
		vMul2.push_back(pMul2[i]);
	return MultiBytesMul(EndianType,vMul1,vMul2,vDecDataOut);
}

/**********************************************
@brief 将给定的两个16进制数据相乘
@param[in] EndianType 决定vAdd1，vAdd2，vDecDataOut的数据组织方式
@param[in] vAdd1	加数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
@param[in] vAdd2	被加数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
@param[out] vDecDataOut	输出的和，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
@return
true	成功
false	失败
************************************************/
bool ComTool::MultiBytesAdd(unsigned int EndianType,QVector<unsigned char>&vAdd1,QVector<unsigned char>&vAdd2,QVector<unsigned char>&vDecDataOut)
{
	int i,LoopCnt;
	bool Ret=true;
	unsigned char AddInc=0,Left=0;
	unsigned short Sum=0;
	QVector<unsigned char>vDecData;
	int Size1,Size2;
	unsigned char *pTmp=NULL,*pTmpAdd1=NULL,*pTmpAdd2=NULL;
	Size1=(int)vAdd1.size();
	Size2=(int)vAdd2.size();
	LoopCnt=Size1>Size2?Size1:Size2;///取
	pTmp=new unsigned char[LoopCnt*2];
	if(pTmp==NULL){
		Ret=false;
		goto __end;
	}
	memset(pTmp,0,LoopCnt*2);
	pTmpAdd1=pTmp;
	pTmpAdd2=pTmp+LoopCnt;
	///将数据转成小端存放
	if(EndianType&ENDIAN_BIG){
		for(i=0;i<Size1;i++){
			pTmpAdd1[Size1-1-i]=vAdd1[i];
		}
		for(i=0;i<Size2;i++){
			pTmpAdd2[Size2-1-i]=vAdd2[i];
		}
	}
	else{
		for(i=0;i<Size1;i++){
			pTmpAdd1[i]=vAdd1[i];
		}
		for(i=0;i<Size2;i++){
			pTmpAdd2[i]=vAdd2[i];
		}
	}

	for(i=0;i<LoopCnt;++i){
		Sum=pTmpAdd1[i]+pTmpAdd2[i]+AddInc;
		AddInc=(Sum>>8)&0xFF;
		Left=Sum&0xFF;
		vDecData.push_back(Left);
	}

	if(AddInc){///需要让最后的进位保存起来
		vDecData.push_back(AddInc);
	}

	if(EndianType&ENDIAN_BIG){
		for(i=(int)vDecData.size()-1;i>=0;i--){
			vDecDataOut.push_back(vDecData[i]);
		}
	}
	else{
		vDecDataOut=vDecData;
	}

__end:
	if(pTmp){
		delete[] pTmp;
	}
	return Ret;
}

/**********************************************
@brief 将给定的两个16进制数据相乘
@param[in] EndianType 决定pAdd1，pAdd2，vDecDataOut的数据组织方式
@param[in] pAdd1	加数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
@param[in] Size1	加数占几个字节
@param[in] pAdd2	被加数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
@param[in] Size2	被加数占几个字节
@param[out] vDecDataOut	输出的和，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
@return
true	成功
false	失败
************************************************/
bool ComTool::MultiBytesAdd(unsigned int EndianType,unsigned char*pAdd1,int Size1,unsigned char*pAdd2,int Size2,QVector<unsigned char>&vDecDataOut)
{
	int i;
	QVector<unsigned char>vAdd1;
	QVector<unsigned char>vAdd2;
	for(i=0;i<Size1;i++)
		vAdd1.push_back(pAdd1[i]);
	for(i=0;i<Size2;i++)
		vAdd2.push_back(pAdd2[i]);
	return MultiBytesAdd(EndianType,vAdd1,vAdd2,vDecDataOut);
}

QString ComTool::GetChksumName( unsigned int uChksumType )
{
	QString strName="";
	switch(uChksumType){
		case 0:
			strName="No Selected";
			break;
		case 1:
			strName="Byte";
			break;
		case 2:
			strName="Word";
			break;
		case 3:
			strName="CRC16";
			break;
		case 4:
			strName="CRC32";
			break;
		case 5:
			strName="CRC64";
			break;
		default:
			strName="Unknown";
			break;
	}
	return strName;
}

void TestComTool(void)
{
	return;
	QVector<unsigned char>vDecData;
	QString strData="987654321012345678901234567890";
	if(ComTool::Str2Dec(strData,ComTool::ENDIAN_BIG,vDecData)==true){
		//vDecData.clear();
	}

	//unsigned char Data[8]={0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	unsigned char Data[64]={0x3D,0xAD,0xD6,0xAC,0xAF,0x11,0xC7,0x1C};
	int Size=(int)vDecData.size();
	for(int i=0;i<Size;i++){
		Data[i]=vDecData[i];
	}
	QString strOut;
	if(ComTool::Dec2Str(ComTool::ENDIAN_BIG,Data,Size,strOut)==true){
	}

	unsigned char Data1[]={0xFF,0x2D,0x46,0x4C};
	unsigned char Data2[]={0xFD,0xAD,0xD6,0xAC};
	vDecData.clear();
	ComTool::MultiBytesAdd(ComTool::ENDIAN_BIG,Data1,sizeof(Data1),Data2,sizeof(Data2),vDecData);

	unsigned char Data3[]={0xFF,0x2D,0x46,0x4c};
	unsigned char Data4[]={0xFD,0xAD,0xD6,0xAc};
	vDecData.clear();
	ComTool::MultiBytesMul(ComTool::ENDIAN_BIG,Data3,sizeof(Data3),Data4,sizeof(Data4),vDecData);

	unsigned int Data5=0xFF2D464c;
	unsigned int Data6=0xFDADD6AC;
	vDecData.clear();
	ComTool::MultiBytesMul(ComTool::ENDIAN_LIT,(unsigned char*)&Data5,sizeof(Data5),(unsigned char*)&Data6,sizeof(Data6),vDecData);
}


//-------------------------------------------------------------------------------------
//Description:
// This function maps a character string to a wide-character (Unicode) string
//
//Parameters:
// lpcszStr: [in] Pointer to the character string to be converted
// lpwszStr: [out] Pointer to a buffer that receives the translated string.
// dwSize: [in] Size of the buffer
// SizeUsed: [out] Size of the buffer be used when translate successfully   ///多少个2个字节单位包含2个字节的0结尾
//
//Return Values:
// 1: Succeed
// 0: Failed
// -1：Need More Buffer
//
//Example:
// MByteToWChar(szA,szW,sizeof(szW)/sizeof(szW[0]),SizeUsed);
//---------------------------------------------------------------------------------------
int ComTool::MByteToWChar(const char* lpcszStr, wchar_t* lpwszStr, unsigned long dwSize,unsigned long& SizeUsed)
{

	if (!lpcszStr || !lpwszStr || dwSize == 0) {
		return 0; // 参数无效
	}

	// 将输入的 C 字符串转换为 QString
	QString qStr = QString::fromUtf8(lpcszStr);

	// 获取 unicode 编码的宽字符字符串的长度（包括终止的 null 字符）
	unsigned long len = qStr.length() + 1;

	// 检查是否提供的缓冲区大小足够
	if (dwSize < len) {
		return -1; // 提供的缓冲区大小不足
	}

	// 将 QString 复制到宽字符缓冲区中
	wcscpy(lpwszStr, reinterpret_cast<const wchar_t*>(qStr.utf16()));

	// 设置实际使用的大小（包括终止的 null 字符）
	SizeUsed = len;

	return 1; // 成功

}


//-------------------------------------------------------------------------------------
//Description:
// This function maps a wide-character string to a new character string
//
//Parameters:
// lpcwszStr: [in] Pointer to the character string to be converted
// lpszStr: [out] Pointer to a buffer that receives the translated string.
// dwSize: [in] Size of the buffer
// SizeUsed: [out] Size of the buffer be used when translate successfully   ///多少个字节单位包含一个字节0结尾

//
//Return Values:
// 1: Succeed
// 0: Failed
// -1：Need More Buffer
//
//Example:
// MByteToWChar(szW,szA,sizeof(szA)/sizeof(szA[0]),SizeUsed);
//---------------------------------------------------------------------------------------
int ComTool::WCharToMByte(const wchar_t* lpcwszStr, char* lpszStr, unsigned long dwSize,unsigned long& SizeUsed)
{
	if (!lpcwszStr || !lpszStr || dwSize == 0) {
		return 0; // 参数无效
	}

	// 将输入的宽字符 C 字符串转换为 QString
	QString qStr = QString::fromWCharArray(lpcwszStr);

	// 将 QString 转换为 UTF-8 编码的 QByteArray
	QByteArray byteArray = qStr.toUtf8();

	// 获取所需的缓冲区大小（包括终止的 null 字符）
	unsigned long len = byteArray.size() + 1;

	// 检查是否提供的缓冲区大小足够
	if (dwSize < len) {
		return -1; // 提供的缓冲区大小不足
	}

	// 将 QByteArray 复制到多字节缓冲区中
	strcpy(lpszStr, byteArray.constData());

	// 设置实际使用的大小（包括终止的 null 字符）
	SizeUsed = len;

	return 1; // 成功
}

//-------------------------------------------------------------------------------------
//Description:
// This function maps a character string to a new utf8 character string
//
//Parameters:
// lpcszStr: [in] Pointer to the character string to be converted
// lpszStr: [out] Pointer to a buffer that receives the translated string.
// dwSize: [in] Size of the lpszStr buffer
// SizeUsed: [out] Size of the buffer be used when translate successfully   ///多少个字节单位包含一个字节0结尾
//
//Return Values:
// 1: Succeed
// 0: Failed
// -1：Need More Buffer
//
//Example:
// MByteToUtf8(szA,szUTF8,sizeof(szUTF8)/sizeof(szUTF8[0]),SizeUsed);
//---------------------------------------------------------------------------------------
int ComTool::MByteToUtf8(const char* lpcszStr, char* lpszStr, unsigned long dwSize,unsigned long& SizeUsed)
{
	int Ret = 1;

	// 获取本地编码（MBCS）的 QTextCodec 对象
	QTextCodec* codec = QTextCodec::codecForLocale();
	if (!codec) {
		return 0; // 错误：找不到编码
	}

	// 将 MBCS 编码的字符串转换为 Unicode（QString）
	QString unicodeString = codec->toUnicode(lpcszStr);

	// 将 Unicode（QString）转换为 UTF-8 编码的 QByteArray
	QByteArray utf8EncodedString = unicodeString.toUtf8();
	SizeUsed = utf8EncodedString.size();

	// 检查提供的缓冲区大小是否足够
	if (dwSize < SizeUsed + 1) {
		return -1; // 错误：缓冲区大小不足
	}

	// 将 UTF-8 编码的字符串复制到输出缓冲区
	memcpy(lpszStr, utf8EncodedString.constData(), SizeUsed);
	lpszStr[SizeUsed] = '\0'; // 空终止字符串

	return Ret; // 成功
}


//-------------------------------------------------------------------------------------
//Description:
// This function maps a utf8  character string to a new utf8 character string
//
//Parameters:
// lpcszStr: [in] Pointer to the character string to be converted
// lpszStr: [out] Pointer to a buffer that receives the translated string.
// dwSize: [in] Size of the lpszStr buffer
// SizeUsed: [out] Size of the buffer be used when translate successfully   ///多少个字节单位包含一个字节0结尾

//
//Return Values:
// 1: Succeed
// 0: Failed
// -1：Need More Buffer
//
//Example:
// Utf8ToMByte(szUTF8,szA,sizeof(szA)/sizeof(szA[0]),SizeUsed);
//---------------------------------------------------------------------------------------
int ComTool::Utf8ToMByte(const char* lpcszStr, char* lpszStr, unsigned long dwSize,unsigned long& SizeUsed)
{
	// Convert UTF-8 to QString
	QString utf8String = QString::fromUtf8(lpcszStr);

	// Get local codec (system encoding)
	QTextCodec* codec = QTextCodec::codecForLocale();
	if (!codec) {
		return 0;  // Error: no codec found
	}

	// Convert QString to local encoding
	QByteArray localEncodedString = codec->fromUnicode(utf8String);
	SizeUsed = localEncodedString.size();

	// Check if provided buffer size is sufficient
	if (dwSize < SizeUsed + 1) {
		return -1;  // Error: buffer size too small
	}

	// Copy the local encoded string to the output buffer
	memcpy(lpszStr, localEncodedString.constData(), SizeUsed);
	lpszStr[SizeUsed] = '\0';  // Null-terminate the string

	return 1;  // Success
}


void ComTool::Split(QString source, QStringList& dest, QString division)
{
	dest = source.split(division);
}

void UnicodeTranslateTest()
{
#if 0
	wchar_t wText[20] = {L"工程"};
	wchar_t wTextGet[20];
	char sText[20]= {0};
	char sTestOrg[20]="工程";
	char sUtf8Text[20]={0};
#else
	wchar_t wText[20] = {L"abcd"};
	wchar_t wTextGet[20];
	char sText[20]= {0};
	char sTestOrg[20]="abcd";
	char sUtf8Text[20]={0};
#endif 
	unsigned long SizeUsed=0;
	ComTool::WCharToMByte(wText,sText,sizeof(sText)/sizeof(sText[0]),SizeUsed);
	ComTool::MByteToWChar(sText,wTextGet,sizeof(wTextGet)/sizeof(wTextGet[0]),SizeUsed);
	ComTool::MByteToUtf8(sTestOrg,sUtf8Text,sizeof(sUtf8Text)/sizeof(sUtf8Text[0]),SizeUsed);
}

int ComTool::hex2Int(QString hexString){

	bool ok;
	int iValue = hexString.toInt(&ok, 16); // 16 进制转换
	if (!ok) {
		iValue = -1; // 转换失败，返回 -1
	}
	return iValue;
}

bool ComTool::isCStringDigit(QString str){
	QRegularExpression re("^\\d+$");
	return re.match(str).hasMatch();
}


unsigned char HexCharToByte(unsigned char ch) {
	if (ch >= '0' && ch <= '9')
		return (unsigned char)(ch - '0');
	if (ch >= 'A' && ch <= 'F')
		return (unsigned char)(ch - 'A' + 10);
	if (ch >= 'a' && ch <= 'f')
		return (unsigned char)(ch - 'a' + 10);
	return -1;
}

// 将十六进制字符串转换为二进制数据
int Str2Hex(const unsigned char* str, unsigned char* pDataBuf, int nDataBufSize) {
	if (nDataBufSize % 2 != 0) {
		return 0;
	}

	int nDataSize = nDataBufSize / 2;
	if (nDataSize > BUFFER_RW_SIZE) {
		return 0;
	}

	for (int i = 0; i < nDataSize; ++i) {
		unsigned char highNibble = HexCharToByte(str[2 * i]);
		unsigned char lowNibble = HexCharToByte(str[2 * i + 1]);
		pDataBuf[i] = (highNibble << 4) | lowNibble;
	}

	return nDataSize; // 返回转换后的数据大小
}