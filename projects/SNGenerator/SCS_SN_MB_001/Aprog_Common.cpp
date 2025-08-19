#include "stdafx.h"
#include "aprog_common.h"



DWORD i_strtohex(const char *str, const int &len)
{
	DWORD nRet	= 0;
	DWORD nAt	= 0;

	for (int i=0; i<len; i++)
	{
		nAt		= str[i];
		switch(nAt) {
		case 'A':
		case 'a':
			nAt	= 0xa; break;
		case 'B':
		case 'b':
			nAt	= 0xb; break;
		case 'C':
		case 'c':
			nAt	= 0xc; break;
		case 'D':
		case 'd':
			nAt	= 0xd; break;
		case 'E':
		case 'e':
			nAt	= 0xe; break;
		case 'F':
		case 'f':
			nAt	= 0xf; break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			nAt	= nAt - 48;
			break;
		default:
			continue;
		}

		for (int j=1; j<len-i; j++)
			nAt	*= 0x10;

		nRet	+= nAt;
	}

	return nRet;
}

int i_strtohex(const char *str, DWORD &dwRtn)
{
	return sscanf(str, "%x", &dwRtn);
}



static int _puthigh(INT8U *place,INT8U data)
{
	int ret=0;
	if(isdigital(data)){///get high 4 bits
		*place |= (data-'0')<<4;
	}
	else if(isalpha_hex(data)){
		if(data>='a'){
			*place |= (data-'a'+0x0A)<<4;
		}
		else{
			*place |= (data-'A'+0x0A)<<4;
		}
	}
	else{
		ret = -1;
	}
	return ret;
}

static int _putlow(INT8U*place,INT8U data)
{
	int ret=0;
	if(isdigital(data)){///get low 4 bits
		*place |= data-'0';
	}
	else if(isalpha_hex(data)){
		if(data>='a'){
			*place |= data-'a'+0x0A;
		}
		else{
			*place |= data-'A'+0x0A;
		}
	}
	else{
		ret = -1;
	}
	return ret;
}


/**********
将str字符串，转换为16进制数，存放到hex中，hex缓冲区的大小由size给出，
最终转换之后占用的字节数由返回值给出。如果返回值小于0则表示失败。
return:
	-1 : 存在非法字符
	-2 : 给定的buffer太小。不能全部转换。
	>=0:转成功的字节数
************/
int Str2Hex(const UCHAR *str,UCHAR *hex,int size)
{
	int reallen=0,buf_len=0;
	int i,ret=0;
	int iseven=0;

	reallen=strlen((char*)str);
	buf_len=(reallen+1)/2;
	if(size<buf_len){
		return -2;
	}
	iseven=(reallen%2)?0:1;

	memset(hex,0,size);

	if(iseven){///the length is even
		for(i=0;i<reallen;i++){
			if(i%2==0){
				ret =_puthigh(hex+i/2,str[i]);
				if(ret==-1)
					goto __end;
			}
			else{
				ret = _putlow(hex+i/2,str[i]);
				if(ret==-1)
					goto __end;
			}
		}
	}
	else{
		for(i=0;i<reallen;i++){///add zero at head
			if(i%2==0){
				ret = _putlow(hex+(i+1)/2,str[i]);
				if(ret==-1)
					goto __end;
			}
			else{
				ret =_puthigh(hex+(i+1)/2,str[i]);
				if(ret==-1)
					goto __end;
			}
		}
	}

__end:
	if(ret!=0){
		return -1;
	}
	return buf_len;
}

/**
convert the hex to string 
hex: the pointer where the digital stored
hexlen : the length of hex
str_len : return the length of the string buffer, end with '\0'
return :
the pointer of the string ,记得需要外部自行调用delete释放内存

example 
hex=F2E32EC4
hexlen=4
return : F2E32EC4
str_len:9
**/
UCHAR* Hex2Str(UCHAR*hex,int hexlen,int *str_len)
{
	int i,ret=0;
	unsigned char* tmpstr=NULL;
	if(hex==NULL || hexlen==0 || str_len==NULL){
		return NULL;
	}

	*str_len = hexlen*2+1;
	tmpstr =new INT8U[*str_len];
	if(tmpstr==NULL){
		ret = -1;
		goto __end;
	}

	for(i=0;i<hexlen;i++){
		if((hex[i]>>4)>=0xA){//high 4 bits
			tmpstr[i*2] = (hex[i]>>4)+'A'-0xA;
		}
		else{
			tmpstr[i*2] = (hex[i]>>4)+'0';
		}

		if((hex[i]&0xF)>=0xA){///low 4 bits
			tmpstr[i*2+1] = (hex[i]&0xF)+'A'-0xA;
		}
		else{
			tmpstr[i*2+1] = (hex[i]&0xF)+'0';
		}
	}

	tmpstr[*str_len-1]=0;

__end:
	if(ret!=0){
		if(tmpstr!=NULL)
			delete []tmpstr;

		tmpstr=NULL;
		*str_len=0;
	}	
	return tmpstr;
}

int PrintDebugInfo(const char *fmt,...)
{
	int ret=0;
	char sprint_buf[1024]; 
	va_list args;
	va_start(args,fmt);  
	vsprintf(sprint_buf,fmt,args); 
	va_end(args); /* 将argp置为NULL */
	strcat(sprint_buf,"\n");
	OutputDebugString(sprint_buf);
	return ret;
}

void SaveDataToFile(CString strDestFile,BYTE*pData,INT Size)
{
	CFile File;
	if(File.Open(strDestFile,CFile::modeCreate|CFile::modeWrite)==TRUE){
		File.Write(pData,Size);
		File.Flush();
		File.Close();
	}
}

int DisplayError(TCHAR * ErrorName,CString&strErrMsg)
{
	DWORD Err = GetLastError();
	LPTSTR lpMessageBuffer = NULL;
	TCHAR StrMsg[512];
	//FormatMessage 将GetLastError函数得到的错误信息（这个错误信息是数字代号）转化成字符串信息的函数。
	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,Err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR)&lpMessageBuffer,0,NULL )){
			_stprintf(StrMsg,_T("%s FAILURE: (0x%08X) %s\n"),ErrorName,Err,(LPTSTR)lpMessageBuffer);
			//ConsolePrint(_T("%s FAILURE: (0x%08X)\n"),ErrorName,Err);
	}
	else{
		_stprintf(StrMsg,_T("%s FAILURE: (0x%08X)\n"),ErrorName,Err);
	}
	if (lpMessageBuffer) 
		LocalFree( lpMessageBuffer ); // Free system buffer
	OutputDebugString(StrMsg);
	strErrMsg.Format("%s",StrMsg);
	SetLastError(Err);
	return FALSE;
}

