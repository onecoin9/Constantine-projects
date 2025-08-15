#pragma once

typedef void (cdecl  *FnRegisterID)(char path[], unsigned char* reg, char msg[], int len);
typedef void (cdecl  *FnGet_Firmware)(char out_sourcing[], char path[], char msg[], char url[], unsigned char* reg,
									  char filename[], int len, int len2, int len3);

typedef void (cdecl  *FnGetTuple_Zigbee) (char path[], unsigned char* reg, char TupleOut[], int len);
typedef void (cdecl *FnGetEncrypTuple)(char product_model[], char path[], unsigned char *reg, char Message[], char TupleOut[], int len, int len2);
typedef void (cdecl  *FnGetTuple_BLE) (char path[], unsigned char* reg, char bleData[], int len);
typedef void (cdecl *FnGetPlatformGetEncrypTuple_BLE)(char product_model[], char path[], unsigned char *reg, char Message[], char TupleOut[], int len, int len2);


typedef struct tagDll{
	FnRegisterID pFnRegisterID;
	FnGet_Firmware pFnGet_Firmware;
	FnGetTuple_Zigbee pFnGetTuple_Zigbee;
	FnGetEncrypTuple pFnGetEncrypTuple;
	FnGetTuple_BLE pGetTuple_BLE;
	FnGetPlatformGetEncrypTuple_BLE pGetPlatformGetEncrypTuple_BLE;
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
	void GetEncrypTuple(char product_model[], char path[], unsigned char *reg, char Message[], char TupleOut[], int len, int len2);
	void GetTuple_BLE(char path[], unsigned char* reg, char bleData[], int len);
	void GetPlatformGetEncrypTuple_BLE(char product_model[], char path[], unsigned char *reg, char Message[], char TupleOut[], int len, int len2);




public:
	bool m_bLoadDll;
private:
	HINSTANCE m_hLib;
	tagDll m_tagDll;
};
