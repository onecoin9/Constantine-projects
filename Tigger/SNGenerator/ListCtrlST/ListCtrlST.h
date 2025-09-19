/**********************************************
Author: sllin
Email:	sllin40@qq.com
**********************************************/
#pragma once


// CListCtrlST
#include <vector>
#include "afxcmn.h"
#include "ListEditST.h"
#include "ComboBoxST.h"

enum eInputType{
	INPUT_DISABLE,		///������ı�
	INPUT_TEXT,			///ͨ���ı��������иı�
	INPUT_COMBOBOX,		///ͨ����Ͽ��ѡ����иı�
	INPUT_DLGFILESEL,	///�ļ�ѡ��
	INPUT_DLGFOLDERSEL, ///ѡ���ļ���
};

typedef struct tagListInput{
	INT ColumnIdx;				///�б������Ӧ��������
	eInputType InputType;		///���������
	INT nTextDataType;			///������ΪINPUT_TEXTʱ���ܹ����յ��ַ��������ͣ��ο� CListEditST�� DATATYPE_STR etc
	UINT dwTextLimit;			///������ΪINPUT_TEXTʱ���ܹ����յ�����ַ�����	
	std::vector<CString>vCmbContent; ///������ΪINPUT_COMBOBOXʱ����Ͽ����г��Ŀ�ѡ��
	CString strFilter;  ///������ΪINPUT_DLGSELFILE,Ϊ�ļ�ɸѡ����Ӧ���ַ���
}LISTINPUT;

/************************************************
@brief �ı������뷢���仯֮��Ļص�����
@param[in] Para		�ⲿ�Ĳ�����RegistSetTextCallBack�ĵڶ�������
@param[in] nRow		�仯���ڵ���
@param[in] nColumn	�仯���ڵ���
@param[in] strText	�µ������ı�
*************************************************/
typedef LRESULT (CALLBACK *FuncSetText)(void *Para,INT nRow,INT nColumn,CString&strText);
typedef CString (CALLBACK *FuncGetTipText)(void *Para,INT nRow,INT nColumn);

/*****************************************
��� nRow��nColumn���ڵĴ���Ϊ�ı�����򣬶���ͨ��RegistGetTextInfoCallBackע���˻ص�����
�������ʾ�ı�������ʱ���ô����л��������ı���������������ַ���
����(UINT)-1�����ú�������Ҫ���ã���ȡ��SetInputTypeAsText���õ���������ַ���Ϊ�趨ֵ
subCmdΪCMD_GETTEXTLIMIT ect������subCmd��ȡ��Ӧ�Ľ��
*******************************************/
#define CMD_GETTEXTLIMIT	(1)
#define CMD_GETTEXTTYPE		(2)
typedef UINT	(CALLBACK*FuncGetTextInfo)(void*Para,INT subCmd,INT nRow,INT nColumn);
///
class CListCtrlST : public CListCtrl
{
	DECLARE_DYNAMIC(CListCtrlST)

public:
	
	CListCtrlST();
	virtual ~CListCtrlST();

	/************************************************************
	@briefע��ص��������������ı�������֮��ͻ���ûص����������ⲿ�ܹ���������
	@param[in] pfnSetText	ע��Ļص�����,����ο�FuncSetText˵��
	@param[in] Para			�ص�����������ⲿ�Զ���������ڵ���pfnSetTextʱ�ش����ú���
	@return
		TRUE	�ɹ�
		FALSE	ʧ��
	**************************************************************/
	BOOL RegistSetTextCallBack(FuncSetText pfnSetText,void *Para);

	BOOL RegistGetTipTextCallBack(FuncGetTipText pfnGetTipText,void *Para);
	BOOL RegistGetTextInfoCallBack(FuncGetTextInfo pfnGetTextLimit,void *Para);

	/************************************************************
	@brief ��ʼ���б���ͷ����ʾ,Ĭ�������
	@param[in] vListHeader	�б�ͷ������ʾ�vector���ж�����ͻᴴ��������
	@param[in] nFormat		���������Ĭ�������
	@return
		TRUE	�ɹ�
		FALSE	ʧ��
	**************************************************************/
	virtual BOOL InitColumnHeader(std::vector<CString>&vListHeader,int nFormat=LVCFMT_LEFT);
	

	/************************************************************
	@brief ׷��һ�м�¼
	@param[in] vItemText	׷�ӵļ�¼��Ϣ����Ҫ�еĸ�����InitColumnHeader�е�vListHeaderһ��
	@return
		TRUE	�ɹ�
		FALSE	ʧ��
	**************************************************************/
	virtual BOOL AppendItem(std::vector<CString>&vItemText);

	/************************************************************
	@brief ɾ��һ�м�¼
	@param[in] nItem ָ���к�
	@return
		TRUE	�ɹ�
		FALSE	ʧ��
	**************************************************************/
	virtual BOOL DeleteItem(INT nItem);

	/************************************************************
	@brief �༭��ʧȥ����֮��Ļص�����
	**************************************************************/
	virtual void DisposeEdit();

	/************************************************************
	@brief ��Ͽ�ʧȥ����֮��Ļص�����
	**************************************************************/
	virtual void DisposeComboBox();

	/************************************************************
	@brief	�����иߣ����ñ�������Ҫ������Դ������List�ؼ���Owner Draw Fixed����ΪTrue��
			������ViewΪReport
	@param[in] nHeight	Ҫ���õ��иߣ���λΪ���أ�Ϊ0��ʾ������������и�
	**************************************************************/
	void SetHeight(INT nHeight);

	/************************************************************
	@brief ָ��ĳ�е�ֵͨ����Ͽ���иı�
	@param[in] ColumnIdx	ָ����ֵ
	@param[in] vContent		��Ͽ�����ʾ��ѡ����
	@return
		TRUE	�ɹ�
		FALSE	ʧ��
	**************************************************************/
	BOOL SetInputTypeAsCombo(INT ColumnIdx,std::vector<CString>&vContent);


	/************************************************************
	@brief ָ��ĳ�е�ֵͨ���༭����иı�
	@param[in] ColumnIdx	ָ����ֵ
	@param[in] DataType		�༭������������ַ��������ͣ��ο� CListEditST�� DATATYPE_STR etc
	@param[in] dwTextLimit
	@return
		TRUE	�ɹ�
		FALSE	ʧ��
	**************************************************************/
	BOOL SetInputTypeAsText(INT ColumnIdx,INT DataType,UINT dwTextLimit);

	/*!
	 * @brief: ����Ϊ�ļ�ѡ������ʱ�� 
	 *
	 * @param[in]: ColumnIdx �к�
	 * @param[in]: strFilter �ļ�ѡ�����Ĺ��������� "Bin(*.bin)|*.bin||",�ɲο�CFileDialog�ĵ��������
	 * @return
	 *	TRUE ��ʾ�ɹ���FALSE��ʾʧ��
	 */
	BOOL SetInputTypeAsFileSelector(INT ColumnIdx,CString strFilter);
	BOOL SetInputTypeAsFolderSelector(INT ColumnIdx);


	BOOL SetTips(INT nRow,INT nColumn,CString& strText);
	
	BOOL AdjuctColumnWidth(INT nRow,INT nColumn);
protected:
	DECLARE_MESSAGE_MAP()
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpMeasureItemStruct);
	/************************************************************
	@brief �ڿͻ���˫������������������Ϣ��Ӧ����
	@param[in] pNMHDR �ο�NMHDR�ṹ��
	@return
	0		�ɹ�
	����ֵ	ʧ��
	**************************************************************/
	virtual LRESULT OnLButtonDblClk(NMHDR *pNMHDR);
private:
	CListEditST m_Edit;
	CComboBoxST m_CmbBox;
	std::vector<LISTINPUT>m_vListInput;
	std::vector<CString>m_vListHeader;
	INT m_ColumNum;
	FuncSetText m_fnSetText;
	void *m_Para;
	FuncGetTipText m_fnGetTipText;
	void *m_GetTipTextPara;
	FuncGetTextInfo m_fnGetTextInfo;
	void *m_GetTextInfoPara;
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	INT m_CurColumn;		///��ǰѡ�����
	INT m_CurRow;			///��ǰѡ�����
	INT m_Height;	///�и߶�
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
protected:
	virtual void PreSubclassWindow();

protected:
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	BOOL OnToolNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);

	void CellHitTest(const CPoint& pt, int& nRow, int& nCol) const;
	bool ShowToolTip(const CPoint& pt) const;
	CString GetToolTipText(int nRow, int nCol);
};


