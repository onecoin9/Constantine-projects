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
	INPUT_DISABLE,		///不允许改变
	INPUT_TEXT,			///通过文本输入框进行改变
	INPUT_COMBOBOX,		///通过组合框的选择进行改变
	INPUT_DLGFILESEL,	///文件选择
	INPUT_DLGFOLDERSEL, ///选择文件夹
};

typedef struct tagListInput{
	INT ColumnIdx;				///列表输入对应操作的列
	eInputType InputType;		///输入的类型
	INT nTextDataType;			///当设置为INPUT_TEXT时，能够接收的字符数据类型，参看 CListEditST的 DATATYPE_STR etc
	UINT dwTextLimit;			///当设置为INPUT_TEXT时，能够接收的最大字符个数	
	std::vector<CString>vCmbContent; ///当设置为INPUT_COMBOBOX时，组合框中列出的可选项
	CString strFilter;  ///当设置为INPUT_DLGSELFILE,为文件筛选器对应的字符串
}LISTINPUT;

/************************************************
@brief 文本框输入发生变化之后的回调函数
@param[in] Para		外部的参数，RegistSetTextCallBack的第二个参数
@param[in] nRow		变化所在的行
@param[in] nColumn	变化所在的列
@param[in] strText	新的输入文本
*************************************************/
typedef LRESULT (CALLBACK *FuncSetText)(void *Para,INT nRow,INT nColumn,CString&strText);
typedef CString (CALLBACK *FuncGetTipText)(void *Para,INT nRow,INT nColumn);

/*****************************************
如果 nRow，nColumn所在的窗格为文本输入框，而且通过RegistGetTextInfoCallBack注册了回调函数
则会在显示文本输入框的时候让窗口有机会设置文本输入框的最大输入字符数
返回(UINT)-1表明该函数不需要设置，会取得SetInputTypeAsText设置的输入最大字符数为设定值
subCmd为CMD_GETTEXTLIMIT ect。根据subCmd获取相应的结果
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
	@brief注册回调函数，这样当文本框输入之后就会调用回调函数，让外部能够做出调整
	@param[in] pfnSetText	注册的回调函数,具体参看FuncSetText说明
	@param[in] Para			回调函数带入的外部自定义参数，在调用pfnSetText时回传给该函数
	@return
		TRUE	成功
		FALSE	失败
	**************************************************************/
	BOOL RegistSetTextCallBack(FuncSetText pfnSetText,void *Para);

	BOOL RegistGetTipTextCallBack(FuncGetTipText pfnGetTipText,void *Para);
	BOOL RegistGetTextInfoCallBack(FuncGetTextInfo pfnGetTextLimit,void *Para);

	/************************************************************
	@brief 初始化列表列头部显示,默认左对齐
	@param[in] vListHeader	列表头部的显示项，vector中有多少项就会创建多少列
	@param[in] nFormat		对齐操作，默认左对齐
	@return
		TRUE	成功
		FALSE	失败
	**************************************************************/
	virtual BOOL InitColumnHeader(std::vector<CString>&vListHeader,int nFormat=LVCFMT_LEFT);
	

	/************************************************************
	@brief 追加一行记录
	@param[in] vItemText	追加的记录信息，需要有的个数和InitColumnHeader中的vListHeader一致
	@return
		TRUE	成功
		FALSE	失败
	**************************************************************/
	virtual BOOL AppendItem(std::vector<CString>&vItemText);

	/************************************************************
	@brief 删除一行记录
	@param[in] nItem 指定行号
	@return
		TRUE	成功
		FALSE	失败
	**************************************************************/
	virtual BOOL DeleteItem(INT nItem);

	/************************************************************
	@brief 编辑框失去焦点之后的回调函数
	**************************************************************/
	virtual void DisposeEdit();

	/************************************************************
	@brief 组合框失去焦点之后的回调函数
	**************************************************************/
	virtual void DisposeComboBox();

	/************************************************************
	@brief	调整行高，调用本函数需要将在资源窗口中List控件的Owner Draw Fixed设置为True，
			并设置View为Report
	@param[in] nHeight	要设置的行高，单位为像素，为0表示根据字体调整行高
	**************************************************************/
	void SetHeight(INT nHeight);

	/************************************************************
	@brief 指定某列的值通过组合框进行改变
	@param[in] ColumnIdx	指定列值
	@param[in] vContent		组合框中显示的选择项
	@return
		TRUE	成功
		FALSE	失败
	**************************************************************/
	BOOL SetInputTypeAsCombo(INT ColumnIdx,std::vector<CString>&vContent);


	/************************************************************
	@brief 指定某列的值通过编辑框进行改变
	@param[in] ColumnIdx	指定列值
	@param[in] DataType		编辑框允许输入的字符数据类型，参看 CListEditST的 DATATYPE_STR etc
	@param[in] dwTextLimit
	@return
		TRUE	成功
		FALSE	失败
	**************************************************************/
	BOOL SetInputTypeAsText(INT ColumnIdx,INT DataType,UINT dwTextLimit);

	/*!
	 * @brief: 设置为文件选择器的时候 
	 *
	 * @param[in]: ColumnIdx 列号
	 * @param[in]: strFilter 文件选择器的过滤器比如 "Bin(*.bin)|*.bin||",可参看CFileDialog的第五个参数
	 * @return
	 *	TRUE 表示成功，FALSE表示失败
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
	@brief 在客户区双击了鼠标左键触发的消息响应函数
	@param[in] pNMHDR 参看NMHDR结构体
	@return
	0		成功
	其他值	失败
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
	INT m_CurColumn;		///当前选择的列
	INT m_CurRow;			///当前选择的行
	INT m_Height;	///行高度
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


