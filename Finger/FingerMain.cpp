// FingerMain.cpp: 实现文件
//

#include "stdafx.h"
#include "Finger.h"
#include "FingerMain.h"
#include "afxdialogex.h"
#include "FingerLogin.h"

// FingerMain 对话框

IMPLEMENT_DYNAMIC(FingerMain, CDialogEx)

FingerMain::FingerMain(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

FingerMain::~FingerMain()
{
}

void FingerMain::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(FingerMain, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &FingerMain::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &FingerMain::OnBnClickedOk)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// FingerMain 消息处理程序


void FingerMain::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
	exit(0);
}


void FingerMain::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	this->ShowWindow(SW_HIDE);
	CDialog::OnCancel();
	FingerLogin dlg;
	dlg.DoModal();
	this->ShowWindow(SW_HIDE);
}


void FingerMain::OnPaint()
{
	//添加背景图片
	//CDialog::OnPaint(); 
	//添加代码对话框背景贴图
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
