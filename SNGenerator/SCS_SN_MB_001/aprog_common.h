#ifndef __APROG_COMMON_H__
#define __APROG_COMMON_H__

typedef unsigned char INT8U;

class ILog{
public:
	virtual void SetProgress(INT CurV,INT Total)=0;
	virtual void PrintLog( INT LogLevel,const char *fmt,... )=0;
};

#define BUFFER_RW_SIZE (1024 * 32)

#define SAFEDEL(a) do { if (a) {delete a; a = NULL;} } while (0)

LPCTSTR GetAppPath();

#define  WM_APROG_MESSAGEBOX 45634
UINT AprgMessageBox(LPCTSTR strOutput, UINT uiFlag = 0);

DWORD i_strtohex(const char *str, const int &len);

int i_strtohex(const char *str, DWORD &dwRtn);

int Str2Hex(const UCHAR *str,UCHAR *hex,int size);
UCHAR* Hex2Str(UCHAR*hex,int hexlen,int *str_len);
int PrintDebugInfo(const char *fmt,...);
int DisplayError(TCHAR * ErrorName,CString&strErrMsg);

INT calc_crc16sum(BYTE* buf,UINT size,WORD *pCRC16Sum);
INT calc_crc16sum_checkvirgin(BYTE* buf,UINT size,WORD *pCRC16Sum,BYTE Virgin,UINT*pIsVirgin);

void SaveDataToFile(CString strDestFile,BYTE*pData,INT Size);

#define isdigital(a)	(((a)<='9' && (a)>='0')?1:0)
#define isupper(a)	(((a)>='A' && (a)<='F')?1:0)
#define islower(a)		(((a)>='a' && (a)<='f')?1:0)
#define isalpha_hex(a)	((((a)>='A' && (a)<='F')||((a)>='a'&&(a)<='f'))?1:0)
#define ishex(a)  ((isdigital(a) || isalpha_hex(a))?1:0)
#endif