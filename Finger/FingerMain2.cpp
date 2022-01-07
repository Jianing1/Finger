// FingerMain2.cpp: 实现文件
//

#include "stdafx.h"
#include "Finger.h"
#include "FingerMain2.h"
#include "afxdialogex.h"
#include "FingerDlg.h"

// FingerMain2 对话框

IMPLEMENT_DYNAMIC(FingerMain2, CDialogEx)

FingerMain2::FingerMain2(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG2, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

FingerMain2::~FingerMain2()
{
}

void FingerMain2::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(FingerMain2, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &FingerMain2::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &FingerMain2::OnBnClickedOk)
END_MESSAGE_MAP()


// FingerMain2 消息处理程序


void FingerMain2::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	this->DestroyWindow();
}


void FingerMain2::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	this->ShowWindow(SW_HIDE);
	CFingerDlg dlg;
	dlg.DoModal();
	this->ShowWindow(SW_HIDE);
}
