#pragma once

typedef void (cdecl  *FnRegisterID)(char path[], unsigned char* reg, char msg[], int len);
typedef void (cdecl  *FnGet_Firmware)(char out_sourcing[], char path[], char msg[], char url[], unsigned char* reg,
									  char filename[], int len, int len2, int len3);

typedef void (cdecl  *FnGetTuple_Zigbee) (char path[], unsigned char* reg, char TupleOut[], int len);



typedef struct tagDll{
	FnRegisterID pFnRegisterID;
	FnGet_Firmware pFnGet_Firmware;
	FnGetTuple_Zigbee pFnGetTuple_Zigbee;
};


class CDllHelp
{
public:
	CDllHelp(void);
	~CDllHelp(void);

	BOOL AttachDll();
	BOOL DetachDll();
	void RegisterID(char path[], unsigned char* reg, char msg[], int len);
	void Get_Firmware(char out_sourcing[], char path[], char msg[], char url[], unsigned char* reg,
		char filename[], int len, int len2, int len3);
	void GetTuple_Zigbee(char path[], unsigned char* reg, char TupleOut[], int len);
public:
	bool m_bLoadDll;
private:
	HINSTANCE m_hLib;
	tagDll m_tagDll;
};
