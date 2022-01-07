// CDlgExec.cpp: 实现文件
//

#include"stdafx.h"
#include "Finger.h"
#include "CDlgExec.h"
#include "afxdialogex.h"


// CDlgExec 对话框

IMPLEMENT_DYNAMIC(CDlgExec, CDialogEx)

CDlgExec::CDlgExec(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG2, pParent)
{

}

CDlgExec::~CDlgExec()
{
}

void CDlgExec::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgExec, CDialogEx)
END_MESSAGE_MAP()


// CDlgExec 消息处理程序
