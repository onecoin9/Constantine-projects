#pragma once
#include "../parser_global.h"

# if defined(POF_LIB)
#  define POF_EXPORT Q_DECL_EXPORT
# else
#  define POF_EXPORT Q_DECL_IMPORT
# endif


#pragma pack(push, 1)

struct POF_HEADER
{
    char file_type[4];	/* usually contains "POF" + '\0' */
    unsigned short byte_order; ///0x0000 表示低字节到高字节， 0xFFFF高字节到低字节， 0x1234,当为0的时候 0x34先
    unsigned short version_number;
    long packet_count;	/* total number of packets */
};

struct PACKET_HEAD
{
    short tag; /* tag number - type of packet */
    long length; /* number of bytes in rest of packet */
};

#pragma pack(pop)
enum TPofTag {
	POFTAG_Creator_ID = 1,
	POFTAG_Device_Name = 2,
	POFTAG_Comment_Text = 3,
	POFTAG_Reserved_4 = 4,
	POFTAG_Security_Bit = 5,
	POFTAG_Logical_Address_and_Data = 6,
	POFTAG_Electrical_Address_and_Data = 7,
	POFTAG_Terminator = 8,
	POFTAG_Device_Symbol_Table = 9,
	POFTAG_Test_Vectors = 10,
	POFTAG_Electrical_Address_and_Constant_Data = 12,
	POFTAG_Element_Count = 14,
	POFTAG_Reserved_15 = 15,
	POFTAG_Programming_Checksum = 16,
	POFTAG_Logical_Address_and_Data_32 = 17,
	POFTAG_Option_Register = 18,
	POFTAG_JTAG_Usercode = 19,
	POFTAG_UFM_Data = 24,
};
class POF_EXPORT PofParser : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit PofParser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);

private:
#define CCITT_CRC 0x8408

	int HandleStrTagInfo(struct PACKET_HEAD& PofPackHead, unsigned char* pTagData, char* StrTag);
	int HandleTerminator(struct PACKET_HEAD& PofPackHead, unsigned char* pTagData, unsigned short CRCCacl);
    unsigned int m_BufferDataOffset;
	char gMagicPof[4];
	void pof_init_crc();
	void pof_compute_crc(unsigned char in_byte);
	void pof_calc_crc(unsigned char* pData, int size);
	unsigned int pof_crc_value();

	unsigned int crc_register; /* global 16-bit shift register 	*/
};
