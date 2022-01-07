// FingerLogin.cpp: 实现文件
//
#include "stdafx.h"
#include "Finger.h"
#include "FingerLogin.h"
#include "afxdialogex.h"
#include "FingerDlg.h"
#include "FingerMain.h"
#include "MainSystem.h"
#include <string>
#include <fstream>
#include "InfoFile.h"
#include <vector>
using namespace std;

// FingerLogin 对话框

IMPLEMENT_DYNAMIC(FingerLogin, CDialogEx)

FingerLogin::FingerLogin(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG2, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

FingerLogin::~FingerLogin()
{
}

void FingerLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_name);
	DDX_Text(pDX, IDC_EDIT4, m_name1);
}


BEGIN_MESSAGE_MAP(FingerLogin, CDialogEx)
	
	ON_BN_CLICKED(IDOK, &FingerLogin::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &FingerLogin::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &FingerLogin::OnBnClickedButton1)
	ON_WM_PAINT()
END_MESSAGE_MAP()

// FingerLogin 消息处理程序

void FingerLogin::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);//将控件数据复制给控件变量
	InfoFile file;
	CString name, pwd;
	int flag = 0;

	int lines = file.CountLines();
	int cnt = 0;
	while(cnt<lines)
	{
		name = file.ReadLine(cnt);
		pwd = file.ReadLine(cnt+1);
		if (name == m_name && pwd == m_name1)
		{
			flag = 1;
			break;
		}
		cnt += 2;
	}
	
	if (flag == 1)
	{
		this->ShowWindow(SW_HIDE);
		MainSystem dlg;
		dlg.DoModal();
		this->ShowWindow(SW_HIDE);
	}
	else
	{
		MessageBox(_T("用户名或密码错误!"));
	}
}


void FingerLogin::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	this->ShowWindow(SW_HIDE);
	CDialog::OnCancel();
	FingerMain dlg;
	dlg.DoModal();
	this->ShowWindow(SW_HIDE);
}


void FingerLogin::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	this->ShowWindow(SW_HIDE);
	CDialog::OnCancel();
	CFingerDlg dlg;
	dlg.DoModal();
	this->ShowWindow(SW_HIDE);
}

void FingerLogin::OnPaint()
{
	CPaintDC   dc(this);
	CRect   rect;
	GetClientRect(&rect);    //获取对话框长宽      
	CDC   dcBmp;             //定义并创建一个内存设备环境
	dcBmp.CreateCompatibleDC(&dc);             //创建兼容性DC
	CBitmap   bmpBackground;
	bmpBackground.LoadBitmap(IDB_BITMAP3);    //载入资源中图片
	BITMAP   m_bitmap;                         //图片变量               
	bmpBackground.GetBitmap(&m_bitmap);       //将图片载入位图中
	 //将位图选入临时内存设备环境
	CBitmap* pbmpOld = dcBmp.SelectObject(&bmpBackground);
	//调用函数显示图片StretchBlt显示形状可变
	dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &dcBmp, 0, 0, m_bitmap.bmWidth, m_bitmap.bmHeight, SRCCOPY);
}
