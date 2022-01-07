// MainSystem.cpp: 实现文件
//

#include "stdafx.h"
#include "Finger.h"
#include "MainSystem.h"
#include "afxdialogex.h"
#include "FingerLogin.h"

// MainSystem 对话框

IMPLEMENT_DYNAMIC(MainSystem, CDialogEx)

MainSystem::MainSystem(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG3, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

MainSystem::~MainSystem()
{
}

void MainSystem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(MainSystem, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &MainSystem::OnBnClickedCancel)
END_MESSAGE_MAP()


// MainSystem 消息处理程序


void MainSystem::OnBnClickedCancel()
{
	this->ShowWindow(SW_HIDE);
	CDialog::OnCancel();
	FingerLogin dlg;
	dlg.DoModal();
	this->ShowWindow(SW_HIDE);
}
