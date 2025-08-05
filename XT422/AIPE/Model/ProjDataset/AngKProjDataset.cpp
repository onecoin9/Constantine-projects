#include "AngKProjDataset.h"
#include "AngKDataBuffer.h"

AngKProjDataset::AngKProjDataset(QObject *parent)
	: QObject(parent)
	, m_pDataBuffer(nullptr)
{
	m_pDataBuffer = new AngKDataBuffer(this);
}

AngKProjDataset::~AngKProjDataset()
{
	if (!m_fileDataJsonVec.empty())
		m_fileDataJsonVec.clear();

	if (!m_eMMCFileDataJsonVec.empty())
		m_eMMCFileDataJsonVec.clear();

	if (!m_operList.empty())
		m_operList.clear();

	if (m_pDataBuffer)
	{
		delete m_pDataBuffer;
		m_pDataBuffer = nullptr;
	}
}

void AngKProjDataset::ClearData()
{
	m_fileDataJsonVec.clear();
	m_fileImportDataJsonSerial.ClearJson();
	m_eMMCFileDataJsonVec.clear();
	m_BPUInfos.clear();
	m_ProjProperty.Clear();
}
