#pragma once


// FingerMain 对话框

class FingerMain : public CDialogEx
{
	DECLARE_DYNAMIC(FingerMain)

public:
	FingerMain(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~FingerMain();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnPaint();
};
