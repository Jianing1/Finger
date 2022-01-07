#pragma once

// FingerLogin 对话框

class FingerLogin : public CDialogEx
{
	DECLARE_DYNAMIC(FingerLogin)

public:
	FingerLogin(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~FingerLogin();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG2 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_name;
	CString m_name1;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnPaint();
};
