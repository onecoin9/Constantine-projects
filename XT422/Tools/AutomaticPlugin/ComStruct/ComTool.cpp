#include "ComTool.h"

#include <tchar.h>  
#include <fstream>
/**********************************************
@brief 将给定的字符串按照10进制进行转换，转换结果为16进制，例如字符串为255,则转换结果过为0xFF
@param[in]	str			需要转换的字符串
@param[in]	EndianType	结果数据存放格式
@param[out] vDecDataOut	转换结果存放在vDecData中,EndianType=ENDIAN_BIG第一个字节为最高字节,否则第一个字节为最低字节
@return
	true	转换成功
	false	包含非法字符，转换失败
************************************************/
bool ComTool::Str2Dec(std::string&str,UINT EndianType,std::vector<uint8_t>&vDecDataOut)
{
	vDecDataOut.clear();
	uint8_t AddInc=0,cData;
	USHORT Tmp=0;
	int i,len,j;
	std::vector<uint8_t>vDecData;
	len=str.length();
	vDecData.push_back(AddInc);
	for(i=0;i<len;i++){
		cData=str.at(i);
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
			vDecData[j]=(uint8_t)Tmp;///写回去
		}
		if(AddInc!=0){//有进位需要再次产生一个
			vDecData.push_back(AddInc);
		}	
	}
	if(EndianType==ENDIAN_BIG){
		for(i=(int)vDecData.size()-1;i>=0;i--){
			vDecDataOut.push_back(vDecData[i]);
		}
	}
	else{
		vDecDataOut=vDecData;
	}
	return true;
}


std::string ComTool::GetCurTime(char Seperator)
{
	std::string strTime;
	/*CTime CurTime;
	CurTime = CTime::GetCurrentTime();//获取当前系统时间
	strTime.Format("%04d%02d%02d_%02d%c%02d%c%02d", CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), CurTime.GetHour(),
	Seperator, CurTime.GetMinute(), Seperator, CurTime.GetSecond());*/
	SYSTEMTIME st;
	GetLocalTime(&st);
	char buf[200];
	memset(buf, 0, 200);
	sprintf(buf, "%04d%02d%02d_%02d%c%02d%c%02d_%03d", st.wYear, st.wMonth, st.wDay, st.wHour,
		Seperator, st.wMinute, Seperator, st.wSecond, st.wMilliseconds);
	strTime = buf;
	return strTime;
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
bool ComTool::Dec2Str(UINT EndianType,std::vector<uint8_t>&vDec,std::string& strData)
{
	int Size=(int)vDec.size();
	uint8_t *pTmp=new uint8_t[Size];
	uint8_t Left=0;
	USHORT Tmp=0;
	int i,ZeroCnt=0;
	if(!pTmp){
		return false;
	}
	strData="";
	if(EndianType==ENDIAN_BIG){
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
		strData.insert(0,1,Left+'0');
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
bool ComTool::Dec2Str(UINT EndianType,uint8_t*pData,int Size,std::string& strData)
{
	int i;
	std::vector<uint8_t>vDec;
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
bool ComTool::Str2Hex( std::string&str,UINT EndianType,std::vector<uint8_t>&vDecDataOut)
{
	vDecDataOut.clear();
	uint8_t AddInc=0,cData;
	uint8_t ByteData;
	USHORT Tmp=0;
	int i,len,j;
	std::vector<uint8_t>vDecData;
	len=str.length();
	if((EndianType&PREHEADZERO_NEED)==0){
		vDecData.push_back(AddInc);
		for(i=0;i<len;i++){
			cData=str.at(i);
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
				vDecData[j]=(uint8_t)Tmp;///写回去
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
			cData=str.at(i);
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
bool ComTool::Hex2Str(UINT EndianType,std::vector<uint8_t>&vHex,std::string& strData)
{
	int Size=(int)vHex.size();
	uint8_t *pTmp=new uint8_t[Size];
	uint8_t Left=0;
	USHORT Tmp=0;
	int i,ZeroCnt=0;
	if(!pTmp){
		return false;
	}
	strData="";
	if(EndianType==ENDIAN_BIG){
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
			strData.insert(0,1,Left+'0');
		}
		else{
			strData.insert(0,1,Left-10+'A');
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

bool ComTool::Hex2StrNew(UINT EndianType,std::vector<uint8_t>&vHex,std::string& strData)
{
	int Size=(int)vHex.size();

	strData="";

	if (EndianType==ENDIAN_BIG){
		for (int i = 0; i < Size ; i++){
			std::string strTemp;
			char buf[100];
			memset(buf, 0, 100);
			sprintf(buf, "%02X", vHex[i]);
			strTemp = buf;
			strData += strTemp;
		}
	}else{
		for (int i = Size-1; i >=0 ; --i){
			std::string strTemp;
			char buf[100];
			memset(buf, 0, 100);
			sprintf(buf, "%02X", vHex[i]);
			strTemp = buf;
			strData += strTemp;
		}
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
bool ComTool::Hex2Str(UINT EndianType,uint8_t*pData,int Size,std::string& strData)
{
	int i;
	std::vector<uint8_t>vHex;
	for(i=0;i<Size;i++){
		vHex.push_back(pData[i]);
	}
	return Hex2Str(EndianType,vHex,strData);
}

bool ComTool::Hex2StrNew(UINT EndianType,uint8_t*pData,int Size,std::string& strData)
{
	int i;
	std::vector<uint8_t>vHex;
	for(i=0;i<Size;i++){
		vHex.push_back(pData[i]);
	}
	return Hex2StrNew(EndianType,vHex,strData);
}



void LSBShiftNBytes(uint8_t *pData,int Size,int nBytes)
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
bool ComTool::MultiBytesMul(UINT EndianType,std::vector<uint8_t>&vMul1,std::vector<uint8_t>&vMul2,std::vector<uint8_t>&vDecDataOut)
{
	int i,j,MulCnt,h;
	bool Ret=true;
	uint8_t AddInc=0,Left=0;
	UINT Mul=0;
	int Size1,Size2;
	std::vector<uint8_t>vDecData;
	uint8_t*pResult=NULL,*pResult1=NULL,*pResult2=NULL;
	uint8_t *pTmpMul=NULL,*pTmpMul1=NULL,*pTmpMul2=NULL;
	Size1=(int)vMul1.size();
	Size2=(int)vMul2.size();
	MulCnt=Size1+Size2;
	pTmpMul=new uint8_t[Size1+Size2];
	pResult=new uint8_t[MulCnt*2];///分配两倍的Size1+Size2用于存放临时结果,乘积最大占Size1+Size2个字节
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
	if(EndianType==ENDIAN_BIG){
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
	uint8_t Ch1,Ch2;
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

	if(EndianType==ENDIAN_BIG){
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
bool ComTool::MultiBytesMul(UINT EndianType,uint8_t*pMul1,int Size1,uint8_t*pMul2,int Size2,std::vector<uint8_t>&vDecDataOut)
{
	int i;
	std::vector<uint8_t>vMul1;
	std::vector<uint8_t>vMul2;
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
bool ComTool::MultiBytesAdd(UINT EndianType,std::vector<uint8_t>&vAdd1,std::vector<uint8_t>&vAdd2,std::vector<uint8_t>&vDecDataOut)
{
	int i,LoopCnt;
	bool Ret=true;
	uint8_t AddInc=0,Left=0;
	USHORT Sum=0;
	std::vector<uint8_t>vDecData;
	int Size1,Size2;
	uint8_t *pTmp=NULL,*pTmpAdd1=NULL,*pTmpAdd2=NULL;
	Size1=(int)vAdd1.size();
	Size2=(int)vAdd2.size();
	LoopCnt=Size1>Size2?Size1:Size2;///取
	pTmp=new uint8_t[LoopCnt*2];
	if(pTmp==NULL){
		Ret=false;
		goto __end;
	}
	memset(pTmp,0,LoopCnt*2);
	pTmpAdd1=pTmp;
	pTmpAdd2=pTmp+LoopCnt;
	///将数据转成小端存放
	if(EndianType==ENDIAN_BIG){
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

	if(EndianType==ENDIAN_BIG){
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
bool ComTool::MultiBytesAdd(UINT EndianType,uint8_t*pAdd1,int Size1,uint8_t*pAdd2,int Size2,std::vector<uint8_t>&vDecDataOut)
{
	int i;
	std::vector<uint8_t>vAdd1;
	std::vector<uint8_t>vAdd2;
	for(i=0;i<Size1;i++)
		vAdd1.push_back(pAdd1[i]);
	for(i=0;i<Size2;i++)
		vAdd2.push_back(pAdd2[i]);
	return MultiBytesAdd(EndianType,vAdd1,vAdd2,vDecDataOut);
}

std::string ComTool::GetChksumName( UINT uChksumType )
{
	std::string strName="";
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
	std::vector<uint8_t>vDecData;
	std::string strData="987654321012345678901234567890";
	if(ComTool::Str2Dec(strData,ComTool::ENDIAN_BIG,vDecData)==true){
		//vDecData.clear();
	}

	//uint8_t Data[8]={0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	uint8_t Data[64]={0x3D,0xAD,0xD6,0xAC,0xAF,0x11,0xC7,0x1C};
	int Size=(int)vDecData.size();
	for(int i=0;i<Size;i++){
		Data[i]=vDecData[i];
	}
	std::string strOut;
	if(ComTool::Dec2Str(ComTool::ENDIAN_BIG,Data,Size,strOut)==true){
	}

	uint8_t Data1[]={0xFF,0x2D,0x46,0x4C};
	uint8_t Data2[]={0xFD,0xAD,0xD6,0xAC};
	vDecData.clear();
	ComTool::MultiBytesAdd(ComTool::ENDIAN_BIG,Data1,sizeof(Data1),Data2,sizeof(Data2),vDecData);

	uint8_t Data3[]={0xFF,0x2D,0x46,0x4c};
	uint8_t Data4[]={0xFD,0xAD,0xD6,0xAc};
	vDecData.clear();
	ComTool::MultiBytesMul(ComTool::ENDIAN_BIG,Data3,sizeof(Data3),Data4,sizeof(Data4),vDecData);

	UINT Data5=0xFF2D464c;
	UINT Data6=0xFDADD6AC;
	vDecData.clear();
	ComTool::MultiBytesMul(ComTool::ENDIAN_LIT,(uint8_t*)&Data5,sizeof(Data5),(uint8_t*)&Data6,sizeof(Data6),vDecData);
}

void ComTool::SaveDataToFile(CSerial&lSerial,std::string strDestFile)
{
	//CFile File;
	//if(File.Open(strDestFile,CFile::modeCreate|CFile::modeWrite)==true){
	//	File.Write(lSerial.GetBuffer(),lSerial.length());
	//	File.Flush();
	//	File.Close();
	//}
	std::ofstream outFile(strDestFile, std::ios::binary);
	if (outFile.is_open()) {
		outFile.write((const char*)lSerial.GetBuffer(), lSerial.length());
		outFile.close();
	}
	else {
		// Handle error: unable to open file  
		std::cerr << "Error: Unable to open file " << strDestFile << std::endl;
	}
}

void ComTool::SaveDataToFile(uint8_t*pData,int Size,std::string strDestFile)
{
	//CFile File;
	//if(File.Open(strDestFile,CFile::modeCreate|CFile::modeWrite)==true){
	//	File.Write(pData,Size);
	//	File.Flush();
	//	File.Close();
	//}
	std::ofstream outFile(strDestFile, std::ios::binary);
	if (outFile.is_open()) {
		outFile.write((const char*)pData, Size);
		outFile.close();
	}
	else {
		// Handle error: unable to open file  
		std::cerr << "Error: Unable to open file " << strDestFile << std::endl;
	}
}


bool ComTool::ExtractParaSet(std::string&strParaData,CSerial&lSerial)
{
	bool Ret=true;
	int Cnt,CntBracket=0,i,DataSize=0;
	CHAR Ch;
	CHAR *pTmpData=NULL;
	Cnt=strParaData.length()+1;
	pTmpData=new CHAR[Cnt];
	if(!pTmpData){
		Ret=false;
		goto __end;
	}
	for(i=0;i<strParaData.length();i++){
		Ch=strParaData.at(i);
		if(Ch=='{'){
			CntBracket++;
			if(CntBracket>1){
				pTmpData[DataSize++]=Ch;
			}
		}
		else if(Ch=='}'){
			CntBracket--;
			if(CntBracket==0){
				pTmpData[DataSize++]=0;
			}
			else{
				pTmpData[DataSize++]=Ch;
			}
		}
		else{
			pTmpData[DataSize++]=Ch;
		}
	}
	if(CntBracket!=0){
		Ret=false; goto __end;
	}
	lSerial.ReInit();
	lSerial.SerialInBuff((uint8_t*)pTmpData,DataSize);

__end:
	if(pTmpData){
		delete[] pTmpData;
	}
	return Ret;
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
int ComTool::MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize,DWORD& SizeUsed)
{
	// Get the required size of the buffer that receives the Unicode
	// string.
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, NULL, 0);
	if(dwSize < dwMinSize){
		return -1;
	}
	// Convert headers from ASCII to Unicode.
	SizeUsed=MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, lpwszStr, dwMinSize); 
	return 1;
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
int ComTool::WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize,DWORD& SizeUsed)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(CP_ACP,NULL,lpcwszStr,-1,NULL,0,NULL,false);
	if(dwSize < dwMinSize){
		return -1;
	}
	SizeUsed=WideCharToMultiByte(CP_ACP,NULL,lpcwszStr,-1,lpszStr,dwSize,NULL,false);
	return 1;
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
int ComTool::MByteToUtf8(LPCSTR lpcszStr, LPSTR lpszStr, DWORD dwSize,DWORD& SizeUsed)
{
	int Ret=1;
	DWORD dwMinSize;
	LPWSTR lpwszStr=NULL;
	///MBSC->Unicode
	dwMinSize = MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, NULL, 0);///先转Unicode
	lpwszStr = new WCHAR[dwMinSize];
	if(!lpwszStr){
		Ret=0; goto __end;
	}
	MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, lpwszStr, dwMinSize); 

	///Unicode->UTF8
	dwMinSize = WideCharToMultiByte(CP_UTF8,NULL,lpwszStr,-1,NULL,0,NULL,false);
	if(dwSize < dwMinSize){
		Ret=-1; goto __end;
	}
	SizeUsed=WideCharToMultiByte(CP_UTF8,NULL,lpwszStr,-1,lpszStr,dwSize,NULL,false);

__end:
	if(lpwszStr){
		delete[] lpwszStr;
	}
	return Ret;
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
int ComTool::Utf8ToMByte(LPCSTR lpcszStr, LPSTR lpszStr, DWORD dwSize,DWORD& SizeUsed)
{
	int Ret=1;
	DWORD dwMinSize;
	LPWSTR lpwszStr=NULL;
	///Utf8-->Unicode
	dwMinSize = MultiByteToWideChar (CP_UTF8, 0, lpcszStr, -1, NULL, 0);
	lpwszStr = new WCHAR[dwMinSize];
	if(!lpwszStr){
		Ret=0; goto __end;
	}
	MultiByteToWideChar (CP_UTF8, 0, lpcszStr, -1, lpwszStr, dwMinSize); 

	///Unicode->MBCS
	dwMinSize = WideCharToMultiByte(CP_OEMCP,NULL,lpwszStr,-1,NULL,0,NULL,false);
	if(dwSize < dwMinSize){
		Ret=-1; goto __end;
	}
	SizeUsed=WideCharToMultiByte(CP_OEMCP,NULL,lpwszStr,-1,lpszStr,dwSize,NULL,false);

__end:
	if(lpwszStr){
		delete[] lpwszStr;
	}
	return Ret;
}
static std::string TCHARToString(const TCHAR* tcharStr) {
#ifdef _UNICODE  
	// If TCHAR is wchar_t, convert to std::wstring first  
	std::wstring wstr(tcharStr);
	// Convert std::wstring to std::string (narrowing)  
	std::string str(wstr.begin(), wstr.end());
	return str;
#else  
	// If TCHAR is char, directly convert to std::string  
	return std::string(tcharStr);
#endif  
}
std::string ComTool::GetCurrentPath(void)
{
	TCHAR szFilePath[MAX_PATH + 1];
	TCHAR *pPos = NULL;
	std::string str_url;
	GetModuleFileName(NULL, szFilePath, MAX_PATH);
	pPos = _tcsrchr(szFilePath, _T('\\'));
	if (pPos != NULL) {
		pPos[0] = 0;//删除文件名，只获得路径
		str_url = TCHARToString(szFilePath);
}
	else {
		pPos = _tcsrchr(szFilePath, _T('/'));
		if (pPos == NULL) {
			str_url = "";
		}
		else {
			str_url = TCHARToString(szFilePath);
		}
	}
	return str_url;
}

//void ComTool::Split(std::string source, CStringArray& dest, std::string division)
//{
//	dest.RemoveAll();
//	int pos = 0;
//	while (-1 != pos) {
//		std::string tmp = source.Tokenize(division, pos);
//		if (!tmp.IsEmpty()) {
//			dest.Add(tmp);
//		}
//	}
//}
void ComTool::Split(std::string source, std::vector<std::string>& dest, std::string division) {
	dest.clear(); // Clear the destination vector  

	size_t start = 0;
	size_t end = source.find(division);

	while (end != std::string::npos) {
		std::string token = source.substr(start, end - start);
		if (!token.empty()) {
			dest.push_back(token);
}
		start = end + division.length();
		end = source.find(division, start);
		}

	// Add the last token  
	std::string token = source.substr(start);
	if (!token.empty()) {
		dest.push_back(token);
	}
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
	DWORD SizeUsed=0;
	ComTool::WCharToMByte(wText,sText,sizeof(sText)/sizeof(sText[0]),SizeUsed);
	ComTool::MByteToWChar(sText,wTextGet,sizeof(wTextGet)/sizeof(wTextGet[0]),SizeUsed);
	ComTool::MByteToUtf8(sTestOrg,sUtf8Text,sizeof(sUtf8Text)/sizeof(sUtf8Text[0]),SizeUsed);
}