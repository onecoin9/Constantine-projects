/**********************************************
Author: sllin
Email:	sllin40@qq.com
**********************************************/
#include "ListCtrlST.h"
#include "afxdlgs.h"
#include "FolderSelector.h"

#define WIDTH_MARGIN	(20)		///字符串在窗格中的宽度预留
#define TEXTEDIT_GAP	(4)			///文本编辑框和实际窗格的间隔
#define COMBOBOX_GAP	(40)		///组合框字符串在窗格中的宽度预留
// CListCtrlST

IMPLEMENT_DYNAMIC(CListCtrlST, CListCtrl)

CListCtrlST::CListCtrlST()
{
	m_ColumNum=0;
	m_fnSetText=NULL;
	m_Para=NULL;
	m_fnGetTipText=NULL;
	m_GetTipTextPara=NULL;
	m_fnGetTextInfo=NULL;
}

CListCtrlST::~CListCtrlST()
{
}


BEGIN_MESSAGE_MAP(CListCtrlST, CListCtrl)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CListCtrlST::OnNMDblclk)
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_HSCROLL()
	ON_NOTIFY_EX(TTN_NEEDTEXTA, 0, OnToolNeedText)
	ON_NOTIFY_EX(TTN_NEEDTEXTW, 0, OnToolNeedText)
END_MESSAGE_MAP()



// CListCtrlST 消息处理程序
BOOL CListCtrlST::SetInputTypeAsCombo(INT ColumnIdx,std::vector<CString>&vContent)
{
	if(ColumnIdx>=m_ColumNum){
		return FALSE;
	}
	LISTINPUT&ListInput=m_vListInput[ColumnIdx];
	ListInput.InputType=INPUT_COMBOBOX;
	ListInput.vCmbContent=vContent;
	if(m_CmbBox.GetSafeHwnd()==NULL){
		m_CmbBox.Create(WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL,CRect(0,0,10,100),this,1002);
	}
	return TRUE;
}

// CListCtrlST 消息处理程序
/*!
 * @brief: 设置为文件选择器的时候 
 *
 * @param[in]: ColumnIdx 列号
 * @param[in]: strFilter 文件选择器的过滤器比如 "Bin(*.bin)|*.bin||",可参看CFileDialog的第五个参数
 * @param[in]:
 * @return
 *	TRUE 表示成功，FALSE表示失败
 */
BOOL CListCtrlST::SetInputTypeAsFileSelector(INT ColumnIdx,CString strFilter)
{
	if(ColumnIdx>=m_ColumNum){
		return FALSE;
	}
	LISTINPUT&ListInput=m_vListInput[ColumnIdx];
	ListInput.InputType=INPUT_DLGFILESEL;
	ListInput.strFilter=strFilter;
	return TRUE;
}

BOOL CListCtrlST::SetInputTypeAsFolderSelector(INT ColumnIdx)
{
	if(ColumnIdx>=m_ColumNum){
		return FALSE;
	}
	LISTINPUT&ListInput=m_vListInput[ColumnIdx];
	ListInput.InputType=INPUT_DLGFOLDERSEL;
	return TRUE;
}

/*********************************************************
将某一列的内容设置为可用文本框输入
DataType 指定文本框内的数据类型，参看DATATYPE_STR
TextLimit 指定最长有几个字符输入
**********************************************************/
BOOL CListCtrlST::SetInputTypeAsText(INT ColumnIdx,INT DataType,UINT dwTextLimit)
{
	if(ColumnIdx>=m_ColumNum){
		return FALSE;
	}
	LISTINPUT&ListInput=m_vListInput[ColumnIdx];
	ListInput.InputType=INPUT_TEXT;
	ListInput.nTextDataType=DataType;
	ListInput.dwTextLimit=dwTextLimit;
	if(m_Edit.GetSafeHwnd()==NULL){
		m_Edit.Create(WS_CHILD|WS_CLIPSIBLINGS|WS_EX_TOOLWINDOW,CRect(0,40,10,50),this,1001);
	}
	return TRUE;
}


BOOL CListCtrlST::SetTips(INT nRow,INT nColumn,CString& strText)
{
	LVSETINFOTIP TipInfo;
	TipInfo.cbSize=sizeof(LVSETINFOTIP);
	TipInfo.dwFlags=0;
	TipInfo.pszText=(WCHAR*)strText.GetBuffer(strText.GetLength());
	TipInfo.iItem=nRow;
	TipInfo.iSubItem=nColumn;
	EnableToolTips(TRUE);
	return SetInfoTip(&TipInfo);
}

BOOL CListCtrlST::InitColumnHeader( std::vector<CString>&vListHeader,int nFormat)
{
	INT Size=(INT)vListHeader.size();
	INT i;
	SIZE strSize;
	CDC *pDC=GetDC();
	m_ColumNum=Size;
	LISTINPUT ListInput;
	for(i=0;i<Size;++i){
		GetTextExtentPoint32(pDC->m_hDC,(LPCSTR)vListHeader[i],vListHeader[i].GetLength(),&strSize);
		InsertColumn(i,vListHeader[i],nFormat,strSize.cx+WIDTH_MARGIN,-1);
		ListInput.ColumnIdx=i;
		ListInput.InputType=INPUT_DISABLE;
		ListInput.nTextDataType=CListEditST::DATATYPE_STR;
		ListInput.dwTextLimit=(UINT)-1;
		m_vListInput.push_back(ListInput);
	}
	m_vListHeader=vListHeader;
	return TRUE;
}

BOOL CListCtrlST::DeleteItem(INT nItem)
{
	INT ItemCnt=GetItemCount();
	if(nItem>=ItemCnt){
		return FALSE;
	}
	return CListCtrl::DeleteItem(nItem);
}
///根据内容调整某列的宽度，第一个参数实际没用
BOOL CListCtrlST::AdjuctColumnWidth(INT nRow,INT nColumn)
{
	CString sLabel;
	INT strWidth,HeadWidth,strMaxWidth=0;
	INT i,ListCnt=GetItemCount();
	for(i=0;i<ListCnt;++i){
		sLabel=GetItemText(i,nColumn);
		strWidth=GetStringWidth(sLabel);
		if(strMaxWidth<strWidth){
			strMaxWidth=strWidth;
		}
	}
	HeadWidth=GetStringWidth(m_vListHeader[nColumn]);//GetColumnWidth(nColumn);
	if(strMaxWidth<HeadWidth){
		strMaxWidth=HeadWidth;
	}
	SetColumnWidth(nColumn,strMaxWidth+WIDTH_MARGIN);
	return TRUE;
}


BOOL CListCtrlST::AppendItem( std::vector<CString>&vItemText )
{
	INT i,ItemCnt;
	INT strWidth,CurWidth;
	INT Size=(INT)vItemText.size();
	if(Size!=m_ColumNum){
		return FALSE;
	}
	ItemCnt=GetItemCount();
	InsertItem(ItemCnt,"");
	for(i=0;i<m_ColumNum;++i){
		strWidth=GetStringWidth(vItemText[i]);
		CurWidth=GetColumnWidth(i);
		SetItemText(ItemCnt,i,vItemText[i]);
	}
	return TRUE;
}
void CListCtrlST::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = OnLButtonDblClk(pNMHDR);
}

void CListCtrlST::DisposeEdit()
{ 
	CString sLabel;
	m_Edit.GetWindowText(sLabel);
	this->SetItemText(m_CurRow,m_CurColumn,sLabel);
	AdjuctColumnWidth(m_CurRow,m_CurColumn);
	m_Edit.ShowWindow(SW_HIDE);
	if(m_fnSetText){
		m_fnSetText(m_Para,m_CurRow,m_CurColumn,sLabel);
	}
	return ;
}

void CListCtrlST::DisposeComboBox()
{
	CString sLabel;
	m_CmbBox.GetWindowText(sLabel);
	this->SetItemText(m_CurRow,m_CurColumn,sLabel);
	m_CmbBox.ShowWindow(SW_HIDE);
	if(m_fnSetText){
		m_fnSetText(m_Para,m_CurRow,m_CurColumn,sLabel);
	}
	return ;
}

LRESULT CListCtrlST::OnLButtonDblClk( NMHDR *pNMHDR )
{
	NM_LISTVIEW *pNMListView=(NM_LISTVIEW *)pNMHDR;
	LVHITTESTINFO lvinfo;
	lvinfo.pt = pNMListView->ptAction;
	lvinfo.flags = LVHT_ABOVE;
	UINT dwTextLimit=0,dwDataType=0;
	INT nItem = SubItemHitTest(&lvinfo);
	if(nItem != -1){
		CRect rect;
		CRect editRect;
		m_CurColumn=lvinfo.iSubItem;
		m_CurRow=lvinfo.iItem;
		GetSubItemRect(lvinfo.iItem,lvinfo.iSubItem,LVIR_BOUNDS,rect);
		CString strText;
		if(m_CurColumn>=m_vListInput.size() || m_CurColumn<0){
			MessageBox("The Column Clicked is out of range");
			return 0;
		}
		LISTINPUT& ListInput=m_vListInput[m_CurColumn];
		strText = GetItemText(lvinfo.iItem,lvinfo.iSubItem);			//获取该单元格已存在的文本内容。
		if(ListInput.InputType==INPUT_TEXT){
			editRect=rect;
			editRect.left+=TEXTEDIT_GAP;
			editRect.right=editRect.left+GetColumnWidth(m_CurColumn)-(TEXTEDIT_GAP*2);
			editRect.top+=TEXTEDIT_GAP;
			editRect.bottom-=TEXTEDIT_GAP;
			if(m_fnGetTextInfo){///如果有设置回调函数，则获取当前需要设置的文本宽度
				dwTextLimit=m_fnGetTextInfo(m_GetTextInfoPara,CMD_GETTEXTLIMIT,m_CurRow,m_CurColumn);
				if(dwTextLimit>ListInput.dwTextLimit){///大于设定的值，则使用设定值
					dwTextLimit=ListInput.dwTextLimit;
				}
			}
			else{
				dwTextLimit=ListInput.dwTextLimit;
			}
			m_Edit.SetTextLimit(dwTextLimit);

			if(m_fnGetTextInfo){///如果有设置回调函数，则获取当前需要设置的文本的类型
				dwDataType=m_fnGetTextInfo(m_GetTextInfoPara,CMD_GETTEXTTYPE,m_CurRow,m_CurColumn);
				if(dwDataType==(UINT)-1){///里面没设置
					dwDataType=ListInput.nTextDataType;
				}
			}
			else{
				dwDataType=ListInput.nTextDataType;
			}
			m_Edit.SetDataType(dwDataType);
			m_Edit.MoveWindow(editRect);									//把编辑框移动到该单元格矩形上。
			m_Edit.SetWindowText(strText);									//把单元格原本的内容显示到编辑框上。
			m_Edit.ShowWindow(SW_SHOW);										//显示编辑框。
			m_Edit.SetSel(0,-1);											//全选编辑框的内容。
			m_Edit.SetFocus();												//设置输入焦点在编辑框上。
		}
		else if(ListInput.InputType==INPUT_COMBOBOX){
			INT i,CmbSize=(INT)ListInput.vCmbContent.size();
			INT CurSel,MaxWidth=0,CurWidth;
			m_CmbBox.ResetContent();
			for(i=0;i<CmbSize;++i){
				m_CmbBox.AddString(ListInput.vCmbContent[i]);
				CurWidth=GetStringWidth(ListInput.vCmbContent[i]);
				if(CurWidth>MaxWidth){
					MaxWidth=CurWidth;
				}
			}
			if(MaxWidth+COMBOBOX_GAP>GetColumnWidth(m_CurColumn)){
				SetColumnWidth(m_CurColumn,MaxWidth+COMBOBOX_GAP);
			}
			editRect=rect;
			editRect.right=editRect.left+GetColumnWidth(m_CurColumn);
			editRect.bottom=editRect.top+100;
			m_CmbBox.MoveWindow(editRect);
			CurSel=m_CmbBox.FindString(-1,strText);
			if(CurSel>=0)
				m_CmbBox.SetCurSel(CurSel);
			else
				m_CmbBox.SetCurSel(0);
			m_CmbBox.ShowWindow(SW_SHOW);
			m_CmbBox.SetFocus();
		}
		else if(ListInput.InputType==INPUT_DLGFILESEL){
			CString strFilePath;
			CFileDialog Dlg(TRUE,NULL,NULL,OFN_PATHMUSTEXIST,ListInput.strFilter,this);
			if(Dlg.DoModal()==IDOK){
				strFilePath=Dlg.GetPathName();
				SetItemText(lvinfo.iItem,lvinfo.iSubItem,strFilePath);
				if(m_fnSetText)
					m_fnSetText(m_Para,lvinfo.iItem,lvinfo.iSubItem,strFilePath);
			}
		}
		else if(ListInput.InputType==INPUT_DLGFOLDERSEL){
			CString strFilePath;
			strFilePath=CSelectFolderDlg::Show();
			SetItemText(lvinfo.iItem,lvinfo.iSubItem,strFilePath);
			if(m_fnSetText)
				m_fnSetText(m_Para,lvinfo.iItem,lvinfo.iSubItem,strFilePath);
		}
		UpdateWindow();
	}
	return 0;
}

void CListCtrlST::MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct )
{
	lpMeasureItemStruct->itemHeight = m_Height;  
}

void CListCtrlST::SetHeight(INT nHeight)
{
	CRect rcwin;  
	GetWindowRect(rcwin);  
	WINDOWPOS wp;  
	if(nHeight==0){
		CDC *pDC=GetDC();
		TEXTMETRIC TextMetric;
		GetTextMetrics(pDC->m_hDC,&TextMetric);
		nHeight=TextMetric.tmHeight*3/2;
	}
	m_Height=nHeight;
	wp.hwnd=m_hWnd;  
	wp.cx=rcwin.Width();  
	wp.cy=rcwin.Height();  
	wp.flags=SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER;  	
	SendMessage(WM_WINDOWPOSCHANGED,0,(LPARAM)&wp); 
}

void CListCtrlST::DrawItem(LPDRAWITEMSTRUCT lpMeasureItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpMeasureItemStruct->hDC);    
	LVITEM lvi = {0}; 
	lvi.mask = LVIF_STATE;//|LVIF_IMAGE; 
	lvi.stateMask = LVIS_FOCUSED | LVIS_SELECTED ; 
	lvi.iItem = lpMeasureItemStruct->itemID; 
	BOOL bGet = GetItem(&lvi); 
	//高亮显示
	BOOL bHighlight =((lvi.state & LVIS_DROPHILITED)||((lvi.state & LVIS_SELECTED) && 
		((GetFocus() == this)|| (GetStyle() & LVS_SHOWSELALWAYS)))); 
	// 画文本背景 
	CRect rcBack = lpMeasureItemStruct->rcItem; 
	pDC->SetBkMode(TRANSPARENT); 
	if( bHighlight ){ //如果被选中
		pDC->SetTextColor(RGB(255,255,255)); //文本为白色
		pDC->FillRect(rcBack, &CBrush(RGB(90,162,0))); 
	} 
	else{ 
		pDC->SetTextColor(RGB(0,0,0));       //文本为黑色
		pDC->FillRect(rcBack, &CBrush(RGB(255,255,255))); 
	} 
	if (lpMeasureItemStruct->itemAction & ODA_DRAWENTIRE){ 
		//写文本 
		CString szText; 
		INT nColumn = GetHeaderCtrl()->GetItemCount();//列数
		for (INT i = 0; i < nColumn; i++) { 
			//循环得到文本 
			CRect rcItem; 
			INT CurWidth,strWidth;
			szText = GetItemText( lpMeasureItemStruct->itemID, i ); 

			///根据内容自动调整宽度
			strWidth=GetStringWidth(szText);
			CurWidth=GetColumnWidth(i);
			if(strWidth>CurWidth){
				SetColumnWidth(i,strWidth+WIDTH_MARGIN);
			}
			if ( !GetSubItemRect(lpMeasureItemStruct->itemID, i, LVIR_LABEL, rcItem )) 
				continue; 	
			rcItem.left += 5; rcItem.right -= 1; 
			pDC->DrawText(szText, lstrlen(szText), &rcItem,  DT_LEFT | DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE);
		} 
	} 

}

BOOL CListCtrlST::RegistSetTextCallBack( FuncSetText pfnSetText,void *Para )
{
	m_Para=Para;
	m_fnSetText=pfnSetText;
	return TRUE;
}

void CListCtrlST::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
	if(m_CmbBox.GetSafeHwnd() && m_CmbBox.IsWindowVisible()){
		CRect rect;
		GetSubItemRect(m_CurRow,m_CurColumn,LVIR_BOUNDS,rect);
		m_CmbBox.MoveWindow(rect);
		m_CmbBox.Invalidate(TRUE);
	}
	if(m_Edit.GetSafeHwnd()){
		CRect rect;
		CRect editRect;
		GetSubItemRect(m_CurRow,m_CurColumn,LVIR_BOUNDS,rect);
		editRect=rect;
		editRect.left+=TEXTEDIT_GAP;
		editRect.right=editRect.left+GetColumnWidth(m_CurColumn)-(TEXTEDIT_GAP*2);
		editRect.top+=TEXTEDIT_GAP;
		editRect.bottom-=TEXTEDIT_GAP;
		m_Edit.MoveWindow(editRect);
		m_Edit.Invalidate(TRUE);
	}
}



void CListCtrlST::PreSubclassWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	CListCtrl::PreSubclassWindow();

	// Disable the CToolTipCtrl of CListCtrl so it won't disturb the CWnd tooltip
	GetToolTips()->Activate(FALSE);

	// Activate the standard CWnd tooltip functionality
	VERIFY( EnableToolTips(TRUE) );
}

bool CListCtrlST::ShowToolTip(const CPoint& pt) const
{
	// Lookup up the cell
	int nRow, nCol;
	CellHitTest(pt, nRow, nCol);

	if (nRow!=-1 && nCol!=-1)
		return true;
	else
		return false;
}

INT_PTR CListCtrlST::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	// TODO: 在此添加专用代码和/或调用基类
	CPoint pt(GetMessagePos());
	ScreenToClient(&pt);
	if (!ShowToolTip(pt))
		return -1;

	int nRow, nCol;
	CellHitTest(pt, nRow, nCol);

	//Get the client (area occupied by this control)
	RECT rcClient;
	GetClientRect( &rcClient );

	//Fill in the TOOLINFO structure
	pTI->hwnd = m_hWnd;
	pTI->uId = (UINT) (nRow * 1000 + nCol);
	// Send TTN_NEEDTEXT when tooltip should be shown
	pTI->lpszText = LPSTR_TEXTCALLBACK;
	pTI->rect = rcClient;

	return pTI->uId;
	//return CListCtrl::OnToolHitTest(point, pTI);
}


void CListCtrlST::CellHitTest(const CPoint& pt, int& nRow, int& nCol) const
{
	nRow = -1;
	nCol = -1;

	LVHITTESTINFO lvhti = {0};
	lvhti.pt = pt;
	nRow = ListView_SubItemHitTest(m_hWnd, &lvhti);	// SubItemHitTest is non-const
	nCol = lvhti.iSubItem;
	if (!(lvhti.flags & LVHT_ONITEMLABEL))
		nRow = -1;
}

CString CListCtrlST::GetToolTipText(int nRow, int nCol)
{
	if (nRow!=-1 && nCol!=-1){
		if(m_fnGetTipText){
			return m_fnGetTipText(m_GetTipTextPara,nRow,nCol);
		}	
		else{
			return CString("");
		}
		//return GetItemText(nRow, nCol);	// Cell-ToolTip
	}
	else{
		return CString("");
	}
}

BOOL CListCtrlST::OnToolNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	CPoint pt(GetMessagePos());
	ScreenToClient(&pt);

	int nRow, nCol;
	CellHitTest(pt, nRow, nCol);

	CString tooltip = GetToolTipText(nRow, nCol);
	if (tooltip.IsEmpty())
		return FALSE;

	// Non-unicode applications can receive requests for tooltip-text in unicode
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
		lstrcpyn(pTTTA->szText, static_cast<LPCTSTR>(tooltip), sizeof(pTTTA->szText));
	else
		_mbstowcsz(pTTTW->szText, static_cast<LPCTSTR>(tooltip), 
		sizeof(pTTTW->szText)/sizeof(WCHAR));
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz(pTTTA->szText, static_cast<LPCTSTR>(tooltip), sizeof(pTTTA->szText));
	else
		lstrcpyn(pTTTW->szText, static_cast<LPCTSTR>(tooltip), 
		sizeof(pTTTW->szText)/sizeof(WCHAR));
#endif
	// If wanting to display a tooltip which is longer than 80 characters,
	// one must allocate the needed text-buffer instead of using szText,
	// and point the TOOLTIPTEXT::lpszText to this text-buffer.
	// When doing this, one is required to release this text-buffer again
	return TRUE;
}

BOOL CListCtrlST::RegistGetTipTextCallBack( FuncGetTipText pfnGetTipText,void *Para )
{
	m_fnGetTipText=pfnGetTipText;
	m_GetTipTextPara=Para;
	return TRUE;
}

BOOL CListCtrlST::RegistGetTextInfoCallBack( FuncGetTextInfo pfnGetTextInfo,void *Para )
{
	m_fnGetTextInfo=pfnGetTextInfo;
	m_GetTextInfoPara=Para;
	return TRUE;
}