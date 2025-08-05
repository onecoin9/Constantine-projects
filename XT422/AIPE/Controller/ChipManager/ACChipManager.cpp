#include "ACChipManager.h"
#include "AngKPathResolve.h"
#include "AngkLogger.h"
#include <time.h>

static std::vector<chip> m_vecChipData;
static std::vector<manufacture> m_vecManufactureData;
static std::vector<adapter> m_vecAdapterData;
static std::vector<chiptype> m_vecChiptypeData;

static chip m_chipData;

ACChipManager::ACChipManager() 
{
	InitACChipData();
}

ACChipManager::~ACChipManager()
{

}

std::vector<chip> ACChipManager::OperSelectChipData(QString dbFile)
{
	db_ptr<ISQList> _db = make_db_ptr<ISQList>();
	QString&& db_path = Utils::AngKPathResolve::localDBPath(dbFile);
	if (!_db->Connect(db_path.toStdString())) {
		ALOG_INFO("Open database file fail.", "CU", "--");
		return std::vector<chip>();
	}

	auto sql1 = SQLBuilder()
		.SELECT("chip.id, chip.name AS ChipName, manufacture.name AS Manufacture, TableAdp1.name AS Adapter1, TableAdp2.name AS Adapter2")
		.SELECT("TableAdp3.name AS Adapter3, package.name AS PackageName, chiptype.name AS ChipType, algofile.name AS AlogFile, TableFPGA1.name AS FPGAFile1")
		.SELECT("TableFPGA2.name AS FPGAFile2, appfile.name AS APPFile, TableAdp1.chipid AS AdapterID, TableMstko.name AS MstkFile, chip.buffsize AS BuffSize")
		.SELECT("chip.chipinfo AS ChipInfo, chip.buffinfo AS BufferInfo, chip.debug AS Debug, chip.opcfgmask AS OpertionCfgMask, chip.version AS Version")
		.SELECT("chip.modifyinfo AS ModifyInfo, chip.devid AS DeviceID, chip.drvparam AS DriverParam, chip.sectorsize AS SectorSize, TableHelp.name AS HelpFile, chip.progtype AS ProgType, chip.opcfgattr AS OperAttr")
		.FROM("chip")
		.INNER_JOIN("manufacture on chip.manuid = manufacture.id")
		.INNER_JOIN("adapter AS TableAdp1 ON chip.adapterid = TableAdp1.id")
		.LEFT_OUTER_JOIN("adapter AS TableAdp2 ON chip.adapter2id = TableAdp2.id")
		.LEFT_OUTER_JOIN("adapter AS TableAdp3 ON chip.adapter3id = TableAdp3.id")
		.INNER_JOIN("package ON chip.packid = package.id")
		.INNER_JOIN("chiptype ON chip.typeid = chiptype.id")
		.INNER_JOIN("algofile ON chip.algofileid = algofile.id")
		.INNER_JOIN("fpgafile AS TableFPGA1 ON chip.fpgafileid = TableFPGA1.id")
		.LEFT_OUTER_JOIN("fpgafile AS TableFPGA2 ON chip.fpgafile2id = TableFPGA2.id")
		.INNER_JOIN("appfile ON chip.appfileid = appfile.id")
		.LEFT_OUTER_JOIN("mstkofile AS TableMstko ON TableMstko.id = chip.mstkfileid")
		.LEFT_OUTER_JOIN("spcfile AS TableSPC ON TableSPC.id = chip.spcfileid")
		.LEFT_OUTER_JOIN("helpfile AS TableHelp ON TableHelp.id = chip.helpfileid ")
		.ORDER_BY("manufacture.name ASC")
		.ORDER_BY("chip.name ASC")
		.toString();

	auto&& res = _db->Select<chip>(true, lilaomo::cmp_("", sql1.c_str()), "");	//1.7w数据 查询加输出返回vector用时1s

	return res;
}

std::vector<chip> ACChipManager::GetALLChipData()
{
	return m_vecChipData;
}

std::vector<manufacture> ACChipManager::GetALLManufactureData()
{
	return m_vecManufactureData;
}

std::vector<adapter> ACChipManager::GetALLAdapterData()
{
	return m_vecAdapterData;
}

std::vector<chiptype> ACChipManager::GetALLChipTypeData()
{
	return m_vecChiptypeData;
}

void ACChipManager::SetSelectChipData(int nIndex)
{
	m_chipData = m_vecChipData[nIndex];
}

const chip& ACChipManager::GetChipData()
{
	return m_chipData;
}

void ACChipManager::InitACChipData()
{
	m_vecManufactureData = OperSelectChipDataClassType<manufacture>(CHIPDATA_DATABASE);
	m_vecAdapterData = OperSelectChipDataClassType<adapter>(CHIPDATA_DATABASE);
	m_vecChiptypeData = OperSelectChipDataClassType<chiptype>(CHIPDATA_DATABASE);
	m_vecChipData = OperSelectChipData(CHIPDATA_DATABASE);
}
