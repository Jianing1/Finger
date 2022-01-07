
// FingerDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include"CZKFPEngX.h"//����ָ�Ʋɼ���SDK

#include<direct.h>//for_mkdir()�������ڴ���Ŀ¼
#define TEMP_DIR "temp\\" //��ʱ�ļ��У����ڴ��ָ�����ǰ�������м䴦����
#define DB_DIR   "Database\\"//ָ�ƿ��ļ���
#define DB_INDEX_TXT "Database\\index.txt" //ָ�ƿ������ļ�

//�˴�ͷ�ļ������õ��ı����ͷ���
// 
//CFingerDlg �Ի���
class CFingerDlg : public CDialogEx
{
// ����
public:
	HICON m_hIcon;
	CFingerDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CFingerDlg();
	CZKFPEngX m_zkfpEng;

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FINGER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	DECLARE_EVENTSINK_MAP() //P45
	afx_msg void OnImageReceivedZkfpengx(BOOL FAR * AImageValid);
public:
	afx_msg void OnBnClickedBtnExit();
	CStatic m_staticInfo;
	CStatic m_picCtrl1;
	CStatic m_picCtrl2;

	CString m_name;
	CString m_name1;
	CString m_name2;
	long m_minuCount;

	afx_msg void OnBnClickedBtnConnect();
	//afx_msg void OnBnClickedDisconnect();
	afx_msg void OnBnClickedBtnCapture();
	afx_msg void OnBnClickedBtnDatabase();
	
	afx_msg void OnBnClickedBtnAddImage();
	afx_msg void OnBnClickedBtnMatchimage();
	afx_msg void OnBnClickedBtnRegister();
	afx_msg void OnBnClickedBtnIdentify();
};

//P63
void InitDatabase();//��������ʼ��ָ�ƿ�
void InitFile(char* filename);//��������ʼ���ļ�