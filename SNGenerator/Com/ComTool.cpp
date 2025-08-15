#include "ComTool.h"
/**********************************************
@brief ���������ַ�������10���ƽ���ת����ת�����Ϊ16���ƣ������ַ���Ϊ255,��ת�������Ϊ0xFF
@param[in]	str			��Ҫת�����ַ���
@param[in]	EndianType	������ݴ�Ÿ�ʽ
@param[out] vDecDataOut	ת����������vDecData��,EndianType=ENDIAN_BIG��һ���ֽ�Ϊ����ֽ�,�����һ���ֽ�Ϊ����ֽ�
@return
	TRUE	ת���ɹ�
	FALSE	�����Ƿ��ַ���ת��ʧ��
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
			vDecData[j]=(BYTE)Tmp;///д��ȥ
		}
		if(AddInc!=0){//�н�λ��Ҫ�ٴβ���һ��
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
@brief �����������ݰ���10���ƽ���ת�����
@param[in] EndianType	ָ��vDec��������֯��ʽ
@param[in] vDec			��Ҫת�������ݣ�EndianType=ENDIAN_BIGʱ���ֽ��ڵ�һ���ֽڣ�������ֽ��ڵ�һ���ֽ�
@param[out] strData		������ַ���
@return
TRUE	ת���ɹ�
FALSE	�����Ƿ��ַ���ת��ʧ��
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
		for(i=0;i<Size;i++){///ת�ɴ�˴���ʽ
			pTmp[i]=vDec[Size-1-i];
		}
	}
	////ͳһ�ô�˴���
	while(ZeroCnt!=Size){///ȫ��Ϊ0����ת�����
		ZeroCnt=0;
		Tmp=0;
		Left=0;
		for(i=0;i<Size;i++){
			Tmp+=pTmp[i];
			Left=Tmp%10;
			pTmp[i]=Tmp/10;
			Tmp=Left<<8;///�������ŵ���8λ
		}
		strData.Insert(-1,Left+'0');
		for(i=0;i<Size;i++){
			if(pTmp[i]==0){///ȫ����0���˳�
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
@brief �����������ݰ���10���ƽ���ת�����
@param[in] EndianType	ָ��pData��������֯��ʽ
@param[in] pData	��Ҫת�������ݣ�EndianType=ENDIAN_BIGʱ���ֽ��ڵ�һ���ֽڣ�������ֽ��ڵ�һ���ֽ�
@param[in] Size		תת������ռ�����ֽ�
@param[out] strData	������ַ���
@return
	TRUE	ת���ɹ�
	FALSE	�����Ƿ��ַ���ת��ʧ��
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
/// ������ǰ��0
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
@brief ���������ַ�������16���ƽ���ת����ת�����Ϊ16���ƣ������ַ���Ϊ255,��ת�������Ϊ0x255
@param[in]	str			��Ҫת�����ַ���
@param[in]	EndianType	������ݴ�Ÿ�ʽ
@param[out] vDecDataOut	ת����������vDecData��,EndianType=ENDIAN_BIG��һ���ֽ�Ϊ����ֽ�,�����һ���ֽ�Ϊ����ֽ�
@return
	TRUE	ת���ɹ�
	FALSE	�����Ƿ��ַ���ת��ʧ��
ע�⣺�ú��������ǰ��0
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
			vDecData[j]=(BYTE)Tmp;///д��ȥ
		}
		if(AddInc!=0){//�н�λ��Ҫ�ٴβ���һ��
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
@brief �����������ݰ���16���ƽ���ת�����
@param[in] EndianType	����vHex��������֯��ʽ ΪENDIAN_BIGʱ���ֽ��ڵ�һ���ֽڣ�������ֽ��ڵ�һ���ֽ�
@param[in] vHex			��Ҫת��������
@param[out] strData		������ַ���
@return
TRUE	ת���ɹ�
FALSE	�����Ƿ��ַ���ת��ʧ��
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
		for(i=0;i<Size;i++){///ת�ɴ�˴���ʽ
			pTmp[i]=vHex[Size-1-i];
		}
	}
	///ͳһ�ô�˴���
	while(ZeroCnt!=Size){///ȫ��Ϊ0����ת�����
		ZeroCnt=0;
		Tmp=0;
		Left=0;
		for(i=0;i<Size;i++){
			Tmp+=pTmp[i];
			Left=Tmp%16;
			pTmp[i]=Tmp/16;
			Tmp=Left<<8;///�������ŵ���8λ
		}
		if(Left<10){
			strData.Insert(-1,Left+'0');
		}
		else{
			strData.Insert(-1,Left-10+'A');
		}
		for(i=0;i<Size;i++){
			if(pTmp[i]==0){///ȫ����0���˳�
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
@brief �����������ݰ���16���ƽ���ת�����
@param[in] pData	��Ҫת�������ݣ����ֽ��ڵ�һ���ֽ�
@param[in] Size		תת������ռ�����ֽ�
@param[out] strData	������ַ���
@return
TRUE	ת���ɹ�
FALSE	�����Ƿ��ַ���ת��ʧ��
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
	for(i=0;i<nBytes;i++){///����λ��0
		pData[i]=0;
	}
}

/**********************************************
@brief ������������16�����������
@param[in] EndianType ����vMul1��vMul2��vDecDataOut��������֯��ʽ
@param[in] vMul1	������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
@param[in] vMul2	��������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
@param[out] vDecDataOut	����ĳ˻���EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
@return
TRUE	�ɹ�
FALSE	ʧ��
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
	pResult=new BYTE[MulCnt*2];///����������Size1+Size2���ڴ����ʱ���,�˻����ռSize1+Size2���ֽ�
	if(pTmpMul==NULL || pResult==NULL){
		Ret=FALSE;
		goto __end;
	}
	///������ת��С�˴��
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
	////ͳһ��С�˴���
	BYTE Ch1,Ch2;
	for(j=0;j<Size2;j++){
		Ch2=pTmpMul2[j];
		AddInc=0;
		for(i=0;i<Size1;i++){///���ֽڳ���1�ֽ�
			Ch1=pTmpMul1[i];
			Mul=Ch1*Ch2+AddInc;
			AddInc=(Mul>>8)&0xFF;
			Left=Mul&0xFF;
			pResult2[i]=Left;
		}
		if(AddInc){
			pResult2[i]=AddInc;
		}
		LSBShiftNBytes(pResult2,MulCnt,j);///���ݳ����ֽڵ�λ�ý��г˻��ֽ�ƫ��
		MultiBytesAdd(ENDIAN_LIT,pResult1,MulCnt,pResult2,MulCnt,vDecData);
		memset(pResult1,0,MulCnt);
		memset(pResult2,0,MulCnt);
		for(h=0;h<(INT)vDecData.size();h++){///Ϊ���´εļ��㽫������浽pResult1�У�
			pResult1[h]=vDecData[h];
		}
	}

	for(i=MulCnt-1;i>=0;i--){///ȥ��ǰ���0
		if(pResult1[i]!=0){
			break;
		}
	}

	if(i<0){///�˻�Ϊ0,��ֻ����һ���ֽڵ�0
		vDecDataOut.push_back(pResult1[0]); 
	}

	if(EndianType==ENDIAN_BIG){
		for(;i>=0;i--){///��ʼ��Ž��
			vDecDataOut.push_back(pResult1[i]);
		}
	}
	else{
		INT ByteCnt=i;
		for(i=0;i<=ByteCnt;i++){///��ʼ��Ž��
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
@brief ������������16�����������
@param[in] EndianType ����pMul1��pMul2��vDecDataOut��������֯��ʽ
@param[in] pMul1	������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
@param[in] Size1	����ռ�����ֽ�
@param[in] pMul2	��������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
@param[in] Size2	������ռ�����ֽ�
@param[out] vDecDataOut	����ĳ˻���EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
@return
TRUE	�ɹ�
FALSE	ʧ��
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
@brief ������������16�����������
@param[in] EndianType ����vAdd1��vAdd2��vDecDataOut��������֯��ʽ
@param[in] vAdd1	������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
@param[in] vAdd2	��������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
@param[out] vDecDataOut	����ĺͣ�EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
@return
TRUE	�ɹ�
FALSE	ʧ��
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
	LoopCnt=Size1>Size2?Size1:Size2;///ȡ
	pTmp=new BYTE[LoopCnt*2];
	if(pTmp==NULL){
		Ret=FALSE;
		goto __end;
	}
	memset(pTmp,0,LoopCnt*2);
	pTmpAdd1=pTmp;
	pTmpAdd2=pTmp+LoopCnt;
	///������ת��С�˴��
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

	if(AddInc){///��Ҫ�����Ľ�λ��������
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
@brief ������������16�����������
@param[in] EndianType ����pAdd1��pAdd2��vDecDataOut��������֯��ʽ
@param[in] pAdd1	������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
@param[in] Size1	����ռ�����ֽ�
@param[in] pAdd2	��������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
@param[in] Size2	������ռ�����ֽ�
@param[out] vDecDataOut	����ĺͣ�EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
@return
TRUE	�ɹ�
FALSE	ʧ��
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
