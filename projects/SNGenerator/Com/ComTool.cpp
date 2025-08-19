#include "ComTool.h"
/**********************************************
@brief 将给定的字符串按照10进制进行转换，转换结果为16进制，例如字符串为255,则转换结果过为0xFF
@param[in]	str			需要转换的字符串
@param[in]	EndianType	结果数据存放格式
@param[out] vDecDataOut	转换结果存放在vDecData中,EndianType=ENDIAN_BIG第一个字节为最高字节,否则第一个字节为最低字节
@return
	TRUE	转换成功
	FALSE	包含非法字符，转换失败
************************************************/
BOOL ComTool::Str2Dec(CString&str,UINT EndianType,std::vector<BYTE>&vDecDataOut)
{
	vDecDataOut.clear();
	BYTE AddInc=0,cData;
	USHORT Tmp=0;
	INT i,len,j;
	std::vector<BYTE>vDecData;
	len=str.GetLength();
	vDecData.push_back(AddInc);
	for(i=0;i<len;i++){
		cData=str.GetAt(i);
		if(!isdigit(cData)){
			return FALSE;
		}
		for(j=0;j<(INT)vDecData.size();j++){
			if(j==0){
				Tmp=vDecData[j]*10+(cData-'0');
			}
			else{
				Tmp=vDecData[j]*10+AddInc;
			}
			AddInc=(Tmp>>8)&0xFF;
			vDecData[j]=(BYTE)Tmp;///写回去
		}
		if(AddInc!=0){//有进位需要再次产生一个
			vDecData.push_back(AddInc);
		}	
	}
	if(EndianType==ENDIAN_BIG){
		for(i=(INT)vDecData.size()-1;i>=0;i--){
			vDecDataOut.push_back(vDecData[i]);
		}
	}
	else{
		vDecDataOut=vDecData;
	}
	return TRUE;
}

/**********************************************
@brief 将给定的数据按照10进制进行转换输出
@param[in] EndianType	指定vDec的数据组织方式
@param[in] vDec			需要转换的数据，EndianType=ENDIAN_BIG时高字节在第一个字节，否则低字节在第一个字节
@param[out] strData		输出的字符串
@return
TRUE	转换成功
FALSE	包含非法字符，转换失败
************************************************/
BOOL ComTool::Dec2Str(UINT EndianType,std::vector<BYTE>&vDec,CString& strData)
{
	INT Size=(INT)vDec.size();
	BYTE *pTmp=new BYTE[Size];
	BYTE Left=0;
	USHORT Tmp=0;
	INT i,ZeroCnt=0;
	if(!pTmp){
		return FALSE;
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
		strData.Insert(-1,Left+'0');
		for(i=0;i<Size;i++){
			if(pTmp[i]==0){///全部是0则退出
				ZeroCnt++;
			}
		}
	}

	if(pTmp){
		delete[] pTmp;
	}
	return TRUE;
}

/**********************************************
@brief 将给定的数据按照10进制进行转换输出
@param[in] EndianType	指定pData的数据组织方式
@param[in] pData	需要转换的数据，EndianType=ENDIAN_BIG时高字节在第一个字节，否则低字节在第一个字节
@param[in] Size		转转的数据占几个字节
@param[out] strData	输出的字符串
@return
	TRUE	转换成功
	FALSE	包含非法字符，转换失败
************************************************/
BOOL ComTool::Dec2Str(UINT EndianType,BYTE*pData,INT Size,CString& strData)
{
	INT i;
	std::vector<BYTE>vDec;
	for(i=0;i<Size;i++){
		vDec.push_back(pData[i]);
	}
	return Dec2Str(EndianType,vDec,strData);
}
/// 不忽略前导0
BOOL ComTool::Str2HexNoTruncate( CString&str,UINT EndianType,std::vector<BYTE>&vDecDataOut)
{
	vDecDataOut.clear();
	BYTE Data=0,cData;
	USHORT Tmp=0;
	INT i,len,j=0;
	std::vector<BYTE>vDecData;
	len=str.GetLength();
	for(i=len-1;i>=0;i--){
		cData=str.GetAt(i);
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
			return FALSE;
		}
		if(j%2==0){
			Data=cData;
		}
		else{
			Data = Data| (cData<<4);
			vDecData.push_back(Data);
		}
		j++;
	}

	if(EndianType==ENDIAN_BIG){
		for(i=(INT)vDecData.size()-1;i>=0;i--){
			vDecDataOut.push_back(vDecData[i]);
		}
	}
	else{
		vDecDataOut=vDecData;
	}
	return TRUE;
}

/**********************************************
@brief 将给定的字符串按照16进制进行转换，转换结果为16进制，例如字符串为255,则转换结果过为0x255
@param[in]	str			需要转换的字符串
@param[in]	EndianType	结果数据存放格式
@param[out] vDecDataOut	转换结果存放在vDecData中,EndianType=ENDIAN_BIG第一个字节为最高字节,否则第一个字节为最低字节
@return
	TRUE	转换成功
	FALSE	包含非法字符，转换失败
注意：该函数会忽略前导0
************************************************/
BOOL ComTool::Str2Hex( CString&str,UINT EndianType,std::vector<BYTE>&vDecDataOut)
{
	vDecDataOut.clear();
	BYTE AddInc=0,cData;
	USHORT Tmp=0;
	INT i,len,j;
	std::vector<BYTE>vDecData;
	len=str.GetLength();
	vDecData.push_back(AddInc);
	for(i=0;i<len;i++){
		cData=str.GetAt(i);
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
			return FALSE;
		}
		for(j=0;j<(INT)vDecData.size();j++){
			if(j==0){
				Tmp=vDecData[j]*16+cData;
			}
			else{
				Tmp=vDecData[j]*16+AddInc;
			}
			AddInc=(Tmp>>8)&0xFF;
			vDecData[j]=(BYTE)Tmp;///写回去
		}
		if(AddInc!=0){//有进位需要再次产生一个
			vDecData.push_back(AddInc);
		}	
	}
	if(EndianType==ENDIAN_BIG){
		for(i=(INT)vDecData.size()-1;i>=0;i--){
			vDecDataOut.push_back(vDecData[i]);
		}
	}
	else{
		vDecDataOut=vDecData;
	}
	return TRUE;
}

/**********************************************
@brief 将给定的数据按照16进制进行转换输出
@param[in] EndianType	决定vHex的数据组织方式 为ENDIAN_BIG时高字节在第一个字节，否则低字节在第一个字节
@param[in] vHex			需要转换的数据
@param[out] strData		输出的字符串
@return
TRUE	转换成功
FALSE	包含非法字符，转换失败
************************************************/
BOOL ComTool::Hex2Str(UINT EndianType,std::vector<BYTE>&vHex,CString& strData)
{
	INT Size=(INT)vHex.size();
	BYTE *pTmp=new BYTE[Size];
	BYTE Left=0;
	USHORT Tmp=0;
	INT i,ZeroCnt=0;
	if(!pTmp){
		return FALSE;
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
			strData.Insert(-1,Left+'0');
		}
		else{
			strData.Insert(-1,Left-10+'A');
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
	return TRUE;
}

/**********************************************
@brief 将给定的数据按照16进制进行转换输出
@param[in] pData	需要转换的数据，高字节在第一个字节
@param[in] Size		转转的数据占几个字节
@param[out] strData	输出的字符串
@return
TRUE	转换成功
FALSE	包含非法字符，转换失败
************************************************/
BOOL ComTool::Hex2Str(UINT EndianType,BYTE*pData,INT Size,CString& strData)
{
	INT i;
	std::vector<BYTE>vHex;
	for(i=0;i<Size;i++){
		vHex.push_back(pData[i]);
	}
	return Hex2Str(EndianType,vHex,strData);
}

void LSBShiftNBytes(BYTE *pData,INT Size,INT nBytes)
{
	INT i=0,ByteLeft=Size-nBytes;
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
TRUE	成功
FALSE	失败
************************************************/
BOOL ComTool::MultiBytesMul(UINT EndianType,std::vector<BYTE>&vMul1,std::vector<BYTE>&vMul2,std::vector<BYTE>&vDecDataOut)
{
	INT i,j,MulCnt,h;
	BOOL Ret=TRUE;
	BYTE AddInc=0,Left=0;
	UINT Mul=0;
	INT Size1,Size2;
	std::vector<BYTE>vDecData;
	BYTE*pResult=NULL,*pResult1=NULL,*pResult2=NULL;
	BYTE *pTmpMul=NULL,*pTmpMul1=NULL,*pTmpMul2=NULL;
	Size1=(INT)vMul1.size();
	Size2=(INT)vMul2.size();
	MulCnt=Size1+Size2;
	pTmpMul=new BYTE[Size1+Size2];
	pResult=new BYTE[MulCnt*2];///分配两倍的Size1+Size2用于存放临时结果,乘积最大占Size1+Size2个字节
	if(pTmpMul==NULL || pResult==NULL){
		Ret=FALSE;
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
	BYTE Ch1,Ch2;
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
		for(h=0;h<(INT)vDecData.size();h++){///为了下次的计算将结果保存到pResult1中，
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
		INT ByteCnt=i;
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
TRUE	成功
FALSE	失败
************************************************/
BOOL ComTool::MultiBytesMul(UINT EndianType,BYTE*pMul1,INT Size1,BYTE*pMul2,INT Size2,std::vector<BYTE>&vDecDataOut)
{
	INT i;
	std::vector<BYTE>vMul1;
	std::vector<BYTE>vMul2;
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
TRUE	成功
FALSE	失败
************************************************/
BOOL ComTool::MultiBytesAdd(UINT EndianType,std::vector<BYTE>&vAdd1,std::vector<BYTE>&vAdd2,std::vector<BYTE>&vDecDataOut)
{
	INT i,LoopCnt;
	BOOL Ret=TRUE;
	BYTE AddInc=0,Left=0;
	USHORT Sum=0;
	std::vector<BYTE>vDecData;
	INT Size1,Size2;
	BYTE *pTmp=NULL,*pTmpAdd1=NULL,*pTmpAdd2=NULL;
	Size1=(INT)vAdd1.size();
	Size2=(INT)vAdd2.size();
	LoopCnt=Size1>Size2?Size1:Size2;///取
	pTmp=new BYTE[LoopCnt*2];
	if(pTmp==NULL){
		Ret=FALSE;
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
		for(i=(INT)vDecData.size()-1;i>=0;i--){
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
TRUE	成功
FALSE	失败
************************************************/
BOOL ComTool::MultiBytesAdd(UINT EndianType,BYTE*pAdd1,INT Size1,BYTE*pAdd2,INT Size2,std::vector<BYTE>&vDecDataOut)
{
	INT i;
	std::vector<BYTE>vAdd1;
	std::vector<BYTE>vAdd2;
	for(i=0;i<Size1;i++)
		vAdd1.push_back(pAdd1[i]);
	for(i=0;i<Size2;i++)
		vAdd2.push_back(pAdd2[i]);
	return MultiBytesAdd(EndianType,vAdd1,vAdd2,vDecDataOut);
}


void TestComTool(void)
{
	return;
	std::vector<BYTE>vDecData;
	CString strData="987654321012345678901234567890";
	if(ComTool::Str2Dec(strData,ComTool::ENDIAN_BIG,vDecData)==TRUE){
		//vDecData.clear();
	}

	//BYTE Data[8]={0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	BYTE Data[64]={0x3D,0xAD,0xD6,0xAC,0xAF,0x11,0xC7,0x1C};
	INT Size=(INT)vDecData.size();
	for(INT i=0;i<Size;i++){
		Data[i]=vDecData[i];
	}
	CString strOut;
	if(ComTool::Dec2Str(ComTool::ENDIAN_BIG,Data,Size,strOut)==TRUE){
	}

	BYTE Data1[]={0xFF,0x2D,0x46,0x4C};
	BYTE Data2[]={0xFD,0xAD,0xD6,0xAC};
	vDecData.clear();
	ComTool::MultiBytesAdd(ComTool::ENDIAN_BIG,Data1,sizeof(Data1),Data2,sizeof(Data2),vDecData);

	BYTE Data3[]={0xFF,0x2D,0x46,0x4c};
	BYTE Data4[]={0xFD,0xAD,0xD6,0xAc};
	vDecData.clear();
	ComTool::MultiBytesMul(ComTool::ENDIAN_BIG,Data3,sizeof(Data3),Data4,sizeof(Data4),vDecData);

	UINT Data5=0xFF2D464c;
	UINT Data6=0xFDADD6AC;
	vDecData.clear();
	ComTool::MultiBytesMul(ComTool::ENDIAN_LIT,(BYTE*)&Data5,sizeof(Data5),(BYTE*)&Data6,sizeof(Data6),vDecData);
}
