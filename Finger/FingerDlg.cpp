// FingerDlg.cpp : 实现文件
#include "stdafx.h"
#include "Finger.h"
#include "FingerDlg.h"
#include "afxdialogex.h"
#include<iostream>
#include<fstream>
#include<iomanip>
#include<Windows.h>
#include<io.h>
#include "FingerLogin.h"
#include "InfoFile.h"

using namespace std;

/*
代码编写要求：
1. 指定在指导书中的位置，过程性的东西思路不要乱
2. 给出详细注释以备后续出错及时复查
*/


//P167
typedef struct NEIGHBOR
{
	long x;            //横坐标（列坐标）
	long y;            //纵坐标（行坐标）
	long type;         //特征点类型（1-端点，3-分叉点）
	float Theta;       //两点连线角度（弧度）
	float Theta2Ridge; //两点脊线方向夹角（弧度）
	float ThetaThisNibor;//相邻特征点的脊线方向（弧度）
	long distance;     //两点距离（像素数量）
}NEIGHBOR;
//P167
typedef struct MINUTIAE
{
	long x;				//横坐标（列坐标）
	long y;				//纵坐标（行坐标）
	long type;			//特征点类型（1-端点，3-分叉点）
	float theta;		//该点处的脊线方向（弧度）
	NEIGHBOR* neibors;	//相邻特征点序列
}MINUTIAE;

//宏定义各步骤临时位图文件
#define EMPTY "empty.bmp"//空白图
#define STEP_IMG_1 "temp\\Step1_Source.bmp"
#define STEP_TXT_1 "temp\\Step1_Source.txt"
#define STEP_TXT_2 "temp\\Step2_MidFilter.txt"
#define STEP_IMG_2 "temp\\Step2_MidFilter.bmp"
#define STEP_TXT_3 "temp\\Step3_Normalize.txt"
#define STEP_IMG_3 "temp\\Step3_Normalize.bmp"
#define STEP_TXT_4 "temp\\Step4_Direction.txt"
#define STEP_IMG_4 "temp\\Step4_Direction.bmp"
#define STEP_TXT_5 "temp\\Step5_Frequency.txt"
#define STEP_IMG_5 "temp\\Step5_Frequency.bmp"
#define STEP_TXT_6 "temp\\Step6_Mask.txt"
#define STEP_IMG_6 "temp\\Step6_Mask.bmp"
#define STEP_TXT_7 "temp\\Step7_GaborEnhance.txt"
#define STEP_IMG_7 "temp\\Step7_GaborEnhance.bmp"
#define STEP_TXT_8 "temp\\Step8_Binary.txt"
#define STEP_IMG_8 "temp\\Step8_Binary.bmp"
#define STEP_TXT_9 "temp\\Step9_Thinning.txt"
#define STEP_IMG_9 "temp\\Step9_Thinning.bmp"
#define STEP_TXT_10 "temp\\Step10_MinuExtract.txt"
#define STEP_IMG_10 "temp\\Step10_MinuExtract.bmp"
#define STEP_TXT_11 "temp\\Step11_MinuFilter.txt"
#define STEP_IMG_11 "temp\\Step11_MinuFilter.bmp"
#define STEP_IMG_11_MDL "temp\\Step11_MinuFilter_MDL.mdl"
#define STEP_IMG_12 "temp\\Step12_Result.bmp"
#define OUTIMAGE "D:\\Finger\\temp\\outImage.bmp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFingerDlg 对话框

CFingerDlg::CFingerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FINGER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//P47：多字节字符串转换为Unicode宽字符串  
wchar_t *ToWideChar(char *str)
{
	int num = MultiByteToWideChar(0, 0, str, -1, NULL, 0);

	wchar_t *wideStr = new wchar_t[num];

	MultiByteToWideChar(0, 0, str, -1, wideStr, num);

	return wideStr;
}

//删除文件夹
bool DeleteF(const TCHAR * sPath, bool bDelDir)
{
	CFileFind ff;
	BOOL bFound;
	bFound = ff.FindFile(CString(sPath) + "\\*.*");
	while (bFound)
	{
		bFound = ff.FindNextFile();
		CString sFilePath = ff.GetFilePath();

		if (ff.IsDirectory())
		{
			if (!ff.IsDots())
			{
				DeleteF(sFilePath, bDelDir);
			}
		}
		else
		{
			if (ff.IsReadOnly())
			{
				SetFileAttributes(sFilePath, FILE_ATTRIBUTE_NORMAL);
			}
			DeleteFile(sFilePath);
		}
	}
	ff.Close();

	if (bDelDir)//是否删除该文件夹
	{
		SetFileAttributes(sPath, FILE_ATTRIBUTE_NORMAL);  //设置文件夹的属性  
		RemoveDirectory(sPath);
	}
	return true;
}


//点击关闭窗口自动删除文件夹（我不想删了）
CFingerDlg::~CFingerDlg()
{
	CDialog::OnDestroy();

	/*CString path1 = ToWideChar("Database\\");
	CString path2 = ToWideChar("temp\\");
	CString path3 = ToWideChar("capt.bmp");

	DeleteF(path1,true);
	DeleteF(path2,true);
	DeleteFile(path3);*/
}

//P42: 绑定控件
void CFingerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_INFO, m_staticInfo);
	DDX_Control(pDX, IDC_STATIC_IMG_1, m_picCtrl1);
	DDX_Control(pDX, IDC_STATIC_IMG_2, m_picCtrl2);
	DDX_Text(pDX, IDC_EDIT_NAME, m_name);
	DDX_Text(pDX, IDC_EDIT_NAME1, m_name1);
	DDX_Text(pDX, IDC_EDIT_NAME2, m_name2);
	DDX_Control(pDX, IDC_ZKFPENGX1, m_zkfpEng);
}

//按钮触发事件
BEGIN_MESSAGE_MAP(CFingerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_EXIT, &CFingerDlg::OnBnClickedBtnExit)
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CFingerDlg::OnBnClickedBtnConnect)
	//ON_BN_CLICKED(IDC_DISCONNECT, &CFingerDlg::OnBnClickedDisconnect)
	ON_BN_CLICKED(IDC_BTN_CAPTURE, &CFingerDlg::OnBnClickedBtnCapture)
	ON_BN_CLICKED(IDC_BTN_DATABASE, &CFingerDlg::OnBnClickedBtnDatabase)
	ON_BN_CLICKED(IDC_BTN_ADD_IMAGE, &CFingerDlg::OnBnClickedBtnAddImage)
	ON_BN_CLICKED(IDC_BTN_MATCH_iMAGE, &CFingerDlg::OnBnClickedBtnMatchimage)
	ON_BN_CLICKED(IDC_BTN_REGISTER, &CFingerDlg::OnBnClickedBtnRegister)
	ON_BN_CLICKED(IDC_BTN_IDENTIFY, &CFingerDlg::OnBnClickedBtnIdentify)
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CFingerDlg,CDialog)
	ON_EVENT(CFingerDlg,IDC_ZKFPENGX1,8,OnImageReceivedZkfpengx,VTS_PBOOL)
	END_EVENTSINK_MAP()

// CFingerDlg 消息处理程序

BOOL CFingerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	InitDatabase();//创建并初始化指纹库P63

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CFingerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else {
		CDialog::OnPaint(); 
	}

}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CFingerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//P46: 在控件中显示图像
int ShowImageInCtrl(CStatic& picCtrl, char* filename)
{
	//载入图像
	CImage image;
	HRESULT hResult = image.Load(ToWideChar(filename));

	//获得图像参数
	long width = image.GetWidth();
	long height = image.GetHeight();

	//设置显示区域
	CRect rect;
	picCtrl.GetClientRect(&rect);
	CDC* pDc = picCtrl.GetWindowDC();
	SetStretchBltMode(pDc->m_hDC, STRETCH_HALFTONE);


	//显示图像
	image.StretchBlt(pDc->m_hDC, rect, SRCCOPY);

	//更新控件显示
	picCtrl.Invalidate(false);

	//释放变量空间
	image.Destroy();

	picCtrl.ReleaseDC(pDc);

	return 0;
}

//P43: 退出事件；
void CFingerDlg::OnBnClickedBtnExit()
{
	//m_zkfpEng.EndEngine();//停止指纹采集器
	CDialog::OnCancel();
	this->ShowWindow(SW_HIDE);
	FingerLogin dlg;
	dlg.DoModal();
	this->ShowWindow(SW_HIDE);
}

//P44: 启动事件
void CFingerDlg::OnBnClickedBtnConnect()
{
	long z = m_zkfpEng.InitEngine();
	if (z == 0) {
		ShowImageInCtrl(m_picCtrl1, EMPTY);//设置成空白图
		ShowImageInCtrl(m_picCtrl2, EMPTY);//设置成空白图
		MessageBox(_T("采集器连接成功."), _T("提示"));//显示信息	
	}
	else if (z == 1) {
		MessageBox(_T("指纹识别驱动程序加载失败."), _T("提示"));//显示信息
	}
	else if (z == 2) {
		MessageBox(_T("没有连接指纹识别仪."), _T("提示"));
	}
	GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(FALSE);//点击启动后，启动按钮灰色，无法再点击。
}

//P45: 接受图像响应事件函数
void CFingerDlg::OnImageReceivedZkfpengx(BOOL FAR* AImageValid)
{
	HDC hdc = this->GetDC()->m_hDC;

	CRect rect, rect1;
	GetDlgItem(IDC_STATIC_IMG_1)->GetClientRect(&rect);
	long width = rect.Width();
	long height = rect.Height();

	GetDlgItem(IDC_STATIC_IMG_1)->GetWindowRect(&rect1);
	ScreenToClient(&rect1);
	int x = rect1.left;
	int y = rect1.top;

	m_zkfpEng.PrintImageAt(long(hdc), x, y, width, height);

	ShowImageInCtrl(m_picCtrl2, EMPTY);//设置成空白图
	m_staticInfo.SetWindowText(ToWideChar(""));
}

////P48: 停止事件
//void CFingerDlg::OnBnClickedDisconnect()
//{
//	m_zkfpEng.EndEngine();//停止指纹采集器
//	//GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(TRUE);
//}

//P50: 采集事件
void CFingerDlg::OnBnClickedBtnCapture()
{
	CString filename = L"capt.bmp";
	CString filter = L"所有文件(*.*)|*.*||";
	CFileDialog dlg(FALSE, NULL, filename, OFN_HIDEREADONLY, filter, this);
	if (dlg.DoModal() == IDOK)//判断是否按下"保存"按钮
	{
		CString str;
		str = dlg.GetPathName();
		m_zkfpEng.SaveBitmap(str);//保存图像
		MessageBox(str);
	}

}

//P51：获取图像参数信息(为后续处理) 
int ReadBMPImgFilePara(char* filename, long& width, long& height, long& depth)
{
	//载入图像
	CImage image;
	HRESULT hResult = image.Load(ToWideChar(filename));
	if (FAILED(hResult) || image.IsNull())//图像损坏或文件不存在
	{
		return -1;
	}

	//获得图像参数

	width = image.GetWidth();
	height = image.GetHeight();
	depth = image.GetBPP();

	if (depth != 8)
	{
		return -2;
	}

	//释放变量空间
	image.Destroy();

	return 0;
}

//P52：从图像文件读取数据CImage
int ReadBMPImgFileDataq(char* filename, unsigned char* data)
{
	//载入图像
	CImage image;
	HRESULT hResult = image.Load(ToWideChar(filename));
	if (FAILED(hResult) || image.IsNull())//图像损坏或文件不存在
	{
		return -1;
	}

	//获得图像参数

	long width = image.GetWidth();
	long height = image.GetHeight();
	long depth = image.GetBPP();

	if (depth != 8)
	{
		return -2;
	}

	//初始化结果数组
	memset(data, 0, width * height);

	//读取图像数据
	int pitch = image.GetPitch();
	unsigned char* pData1 = (unsigned char*)image.GetBits();
	unsigned char* pData2 = data;
	unsigned char gray = 0;

	unsigned char* pRow1, * pRow2, * pPix1, * pPix2;
	for (int y = 0; y < height; y++)
	{
		pRow1 = pData1 + pitch * y;
		pRow2 = pData2 + width * y;
		for (int x = 0; x < width; x++)
		{
			//获取原图像素的灰度值
			pPix1 = pRow1 + x;
			gray = *pPix1;
			//保存到结果数组
			pPix2 = pRow2 + x;
			*pPix2 = gray;
		}
	}
	image.Destroy();
	return 0;
}

//P55: 用文本文件保存数据(data无符号字符型数据数组)
int SaveDataToTextFile(char* dstFile, unsigned char* data, long width, long height)
{
	//打开目标文件
	ofstream fout(dstFile, ios::out);
	if (!fout)
	{
		return -1;
	}
	int space = 5;
	for (long i = 0; i < height * width; i++)
	{
		fout << setw(space) << int(data[i]);
		if (i % width == (width - 1))
		{
			fout << endl;
		}
	}
	fout.close();
	return 0;
}

//P56: 用文本文件保存数据(data浮点型数据数组)
int SaveDataToTextFile(char* dstFile, float* data, long width, long height)
{
	//打开目标文件
	ofstream fout(dstFile, ios::out);
	if (!fout)
	{
		return -1;
	}
	int preci = 6;
	int space = 16;
	fout.precision(preci);

	for (long i = 0; i < height * width; i++)
	{
		fout << setw(space) << data[i];
		if (i % width == (width - 1))
		{
			fout << endl;
		}
	}

	fout.close();

	return 0;
}

//P61: 清空前一次保存的数据
bool DeleteEntireDir(const TCHAR* sPath, bool bDelDir)
{
	CFileFind ff;
	BOOL bFound;
	bFound = ff.FindFile(CString(sPath) + "\\*.*");
	while (bFound)
	{
		bFound = ff.FindNextFile();
		CString sFilePath = ff.GetFilePath();

		if (ff.IsDirectory())
		{
			if (!ff.IsDots())
			{
				DeleteEntireDir(sFilePath, bDelDir);
			}
		}
		else
		{
			if (ff.IsReadOnly())
			{
				SetFileAttributes(sFilePath, FILE_ATTRIBUTE_NORMAL);
			}
			DeleteFile(sFilePath);
		}
	}
	ff.Close();

	if (bDelDir)//是否删除该文件夹
	{
		SetFileAttributes(sPath, FILE_ATTRIBUTE_NORMAL);  //设置文件夹的属性  
		RemoveDirectory(sPath);
	}
	return true;
}

//P61: 清空前一次保存的数据
bool DeleteFolder(const TCHAR* pszFolderPath)
{
	if (_taccess(pszFolderPath, 0) == 0)
	{
		if (!DeleteEntireDir(pszFolderPath, false))
		{
			return false;
		}
	}
	return true;
}

//P64: 获取指纹库统计信息
void GetDataBaseInfo(char* info)
{
	int pNo = 0;//指纹序号
	char name[MAX_PATH] = { 0 };//登记人姓名
	char srcFile[MAX_PATH] = { 0 };//指纹图像文件
	char mdlFile[MAX_PATH] = { 0 };//指纹特征文件
	FILE* index = fopen(DB_INDEX_TXT, "r");//打开索引文件
	while (!feof(index))
	{
		fscanf(index, "%d %s %s %s\n", &pNo, srcFile, mdlFile, name);
	}
	fclose(index);//关闭文件
	//统计指纹库信息
	sprintf(info, "当前指纹库中共有%d条记录", pNo);

}

void CFingerDlg::OnBnClickedBtnDatabase()
{
	// TODO: 在此添加控件通知处理程序代码
	char info[MAX_PATH] = { 0 };//初始化显示信息
	GetDataBaseInfo(info);//获取指纹库信息
	m_staticInfo.SetWindowText(ToWideChar(info));//显示指纹库信息
}

//P63: 创建并初始化指纹库 
void InitDatabase()
{
	_mkdir(TEMP_DIR);//创建临时文件夹
	_mkdir(DB_DIR);  //创建指纹库文件夹
	InitFile(DB_INDEX_TXT);//创建索引文件
}

//P63: 创建并初始化文件
void InitFile(char* filename)
{
	FILE* index = fopen(filename, "w");//若文件不存在则创建，若已经存在则清空内容
	fclose(index);//关闭文件
}

//P72: 从文本文件读取无符号字符类型的图像数据
int ReadDataFromTextFile(char* srcFile, unsigned char* data, long lWidth, long lHeight)
{
	//打开源文件
	ifstream fin(srcFile, ios::in);
	if (!fin) {
		return -1;
	}

	//读取数据
	int d = 0;
	for (long i = 0; i < lHeight * lWidth; i++)
	{
		fin >> d;
		data[i] = (unsigned char)d;
	}
	fin.close();
	return 0;
}

//P72: 从文本文件读取浮点数类型的其他数据
int ReadDataFromTextFile(char* srcFile, float* data, long lWidth, long lHeight)
{
	//打开源文件
	ifstream fin(srcFile, ios::in);
	if (!fin) {
		return -1;
	}

	//读取数据
	for (long i = 0; i < lHeight * lWidth; i++)
	{
		fin >> data[i];
	}
	fin.close();
	return 0;
}

//P78: 数组排序
void Sort(unsigned char* data, long dsize)
{
	unsigned char temp = 0;
	for (long i = 0; i < dsize; i++)
	{
		for (long j = dsize - 1; j > i; j--)
		{
			if (data[j] < data[j - 1])//升序排列
			{
				temp = data[j];
				data[j] = data[j - 1];
				data[j - 1] = temp;
			}
		}
	}
}
//P73: 中值滤波算法 
int MidFilter(unsigned char* ucImg, unsigned char* ucDstImg, long lWidth, long lHeight)
{
	//Step1:结果图像数据初始化
	memset(ucDstImg, 0, lWidth * lHeight);
	//Step2:中心区域滤波（使用3*3领域）
	unsigned char* pUp, * pDown, * pImg;//用3*3领域的3个图像数据指针
	unsigned char x[9];//3*3领域数组
	for (long i = 1; i < lHeight - 1; i++)
	{
		//初始化领域指针
		pUp = ucImg + (i - 1) * lWidth;
		pImg = ucImg + i * lWidth;
		pDown = ucImg + (i + 1) * lWidth;

		for (long j = 1; j < lWidth - 1; j++)//遍历第2列到倒数第2列
		{
			//移动领域指针
			pUp++;
			pImg++;
			pDown++;

			//获取3*3领域数据
			x[0] = *(pUp - 1);
			x[1] = *(pImg - 1);
			x[2] = *(pDown - 1);

			x[3] = *pUp;
			x[4] = *pImg;
			x[5] = *pDown;

			x[6] = *(pUp + 1);
			x[7] = *(pImg + 1);
			x[8] = *(pDown + 1);

			//数组排序
			Sort(x, 9);
			//结果图像数据取领域中值
			*(ucDstImg + i * lWidth + j) = x[4];
		}
	}

	//Step3:第1行和最后1行滤波（使用2*3领域）

	//第1行
	pDown = ucImg + lWidth;//领域指针初始化
	for (long j = 1; j < lWidth - 1; j++)
	{
		//获取2*3领域数据
		x[0] = *(ucImg + j - 1);
		x[1] = *(ucImg + j);
		x[2] = *(ucImg + j + 1);

		x[3] = *(pDown + j - 1);
		x[4] = *(pDown + j);
		x[5] = *(pDown + j + 1);


		//数组排序
		Sort(x, 6);

		//结果取中值
		*(ucDstImg + j) = x[3];

	}
	//最后1行(倒数第1行）
	pUp = ucImg + lWidth * (lHeight - 2);//领域指针初始化
	pDown = ucImg + lWidth * (lHeight - 1);//领域指针初始化
	for (long j = 1; j < lWidth - 1; j++)//最后1行遍历第2列到倒数第2列
	{
		//获取2*3领域数据

		x[0] = *(pDown + j - 1);
		x[1] = *(pDown + j);
		x[2] = *(pDown + j + 1);

		x[3] = *(pUp + j - 1);
		x[4] = *(pUp + j);
		x[5] = *(pUp + j + 1);

		//数组排序
		Sort(x, 6);

		//结果取中值
		*(ucDstImg + lWidth * (lHeight - 1) + j) = x[3];
	}
	//Step4:4个角点滤波（2*2领域）
	x[0] = *(ucImg);//获取2*2领域数据
	x[1] = *(ucImg + 1);
	x[2] = *(ucImg + lWidth);
	x[3] = *(ucImg + lWidth + 1);

	Sort(x, 4);
	*(ucDstImg) = x[2];//结果取中值

	//右上角点
	x[0] = *(ucImg + lWidth - 1);//获取2*2领域数据
	x[1] = *(ucImg + lWidth - 2);
	x[2] = *(ucImg + 2 * lWidth - 1);
	x[3] = *(ucImg + 2 * lWidth - 2);
	Sort(x, 4);
	*(ucDstImg + lWidth - 1) = x[2];//结果取中值


   //左下角点
	x[0] = *(ucImg + (lHeight - 1) * lWidth);//获取2*2领域数据
	x[1] = *(ucImg + (lHeight - 2) * lWidth);
	x[2] = *(ucImg + (lHeight - 1) * lWidth + 1);
	x[3] = *(ucImg + (lHeight - 2) * lWidth + 1);
	Sort(x, 4);
	*(ucDstImg + (lHeight - 1) * lWidth) = x[2];//结果取中值

	//右下角点
	x[0] = *(ucImg + (lHeight - 0) * lWidth - 1);//获取2*2领域数据
	x[1] = *(ucImg + (lHeight - 1) * lWidth - 1);
	x[2] = *(ucImg + (lHeight - 0) * lWidth - 2);
	x[3] = *(ucImg + (lHeight - 1) * lWidth - 2);
	Sort(x, 4);
	*(ucDstImg + (lHeight - 0) * lWidth - 1) = x[2];//结果取中值
	return 0;
}


//P82: 将位图数据写入BMP图像文件  
int WriteBMPImgFile(char* dstFileName, unsigned char** pusImgData) {
	//打开文件
	FILE* fp = fopen(dstFileName, "r+b");

	if (!fp) {
		return -1;
	}
	long IWidth, IHeight, imgType;//图像宽度 高度 深度
	long IStartPos = 0;//位图数据区起始位置
	fseek(fp, 10L, SEEK_SET);
	fread((char*)(&IStartPos), 4, 1, fp);//获取位图数据区起始位置（以字节为单位）
	fseek(fp, 18L, SEEK_SET);
	fread((char*)(&IWidth), 4, 1, fp);//获取图像宽度（以像素为单位）
	fread((char*)(&IHeight), 4, 1, fp);//获取图像高度（以像素为单位）
	unsigned short temp;
	fseek(fp, 28L, SEEK_SET);
	fread((char*)(&temp), 2, 1, fp);//获取图像深度（每个像素的位数，以位为单位）
	imgType = temp;
	if (imgType != 8) {//目前仅支持8为BMP位图图像
		return -2;
	}
	//向数据区写入数据
	unsigned char* usImgData = *pusImgData;//待保存数据数组地址
	long IWidthInFile = 0;//文件中每行像素 宽度、长度（以字节为单位）
	if (IWidth % 4 > 0) {
		IWidthInFile = IWidth - IWidth % 4 + 4;
	}
	else {
		IWidthInFile = IWidth;
	}
	for (int i = IHeight - 1; i >= 0; i--) {//从最后一行倒序存储
		fseek(fp, IStartPos, SEEK_SET);//定位行首地址
		fwrite((usImgData + i * IWidth), 1, IWidth, fp);//写入一行数据
		IStartPos += IWidthInFile;//调整行首地址

	}
	//关闭文件
	fclose(fp);
	return 0;
}

//P80: 保存数据到BMP文件  
int SaveDataToImageFile(char* srcFile, char* dstFile, unsigned char* data) {
	//文件复制
	CopyFile(ToWideChar(srcFile), ToWideChar(dstFile), false);
	//写入数据
	WriteBMPImgFile(dstFile, &data);
	return 0;
}

//P80: 保存数据到图像文件           			 
int SaveDataToImageFile(char* srcFile, char* dstFile, float* data, float scale) {
	//读取图像参数
	long IWidth, IHeight, IDepth;//图像宽度 高度 深度
	ReadBMPImgFilePara(srcFile, IWidth, IHeight, IDepth);

	//文件复制
	CopyFile(ToWideChar(srcFile), ToWideChar(dstFile), false);

	//数据转换
	unsigned char* tmpData = new unsigned char[IWidth * IHeight];
	for (int i = 0; i<int(IWidth * IHeight); i++) {
		tmpData[i] = unsigned char((scale * data[i]));
	}

	//写入数据
	WriteBMPImgFile(dstFile, &tmpData);
	//释放内存空间
	delete[] tmpData;
	return 0;
}

//P88：直方图均衡化                  源图数据               结果图数据      图像宽度       高度
int HistoNormalize(unsigned char* ucImg, unsigned char* ucNormImg, long IWidth, long IHeight) {
	//构建源图灰度直方图
	unsigned long Histogram[256];
	memset(Histogram, 0, 256 * sizeof(long));
	for (long i = 0; i < IHeight; i++) {
		for (long j = 0; j < IWidth; j++) {
			Histogram[ucImg[i * IWidth + j]]++;
		}
	}
	//计算源图的灰度均值和方差
	double dMean = 0;
	for (long i = 0; i < 255; i++) {
		dMean += i * Histogram[i];
	}
	dMean = long(dMean / (IWidth * IHeight));
	double dSigma = 0;
	for (long i = 1; i < 255; i++) {
		dSigma += Histogram[i] * (i - dMean) * (i - dMean);
	}
	dSigma /= (IWidth * IHeight);
	dSigma = sqrt(dSigma);
	//对各像素进行均衡化操作
	double dMean0 = 128, dSigma0 = 128;//预设灰度均值和方差（此处均设为128）
	double dCoeff = dSigma0 / dSigma;//转换系数
	for (long i = 0; i < IHeight; i++) {
		for (long j = 0; j < IWidth; j++) {
			double dVal = ucImg[i * IWidth + j];
			dVal = dMean0 + dCoeff * (dVal - dMean0);
			if (dVal < 0) {
				dVal = 0;
			}
			else if (dVal > 255) {
				dVal = 255;
			}
			ucNormImg[i * IWidth + j] = (unsigned char)dVal;
		}
	}
	return 0;
}

//P95：指纹脊线方向计算          图像数据    脊线方向数据    图像宽          高
int ImgDirection(unsigned char* ucImg, float* fDirc, long IWidth, long IHeight) {
	//定义变量
	const int SEMISIZ = 7;//领域窗口区域半径
	long dx[SEMISIZ * 2 + 1][SEMISIZ * 2 + 1];
	long dy[SEMISIZ * 2 + 1][SEMISIZ * 2 + 1];
	float fx, fy;
	//结果初始化
	memset(fDirc, 0, IWidth * IHeight * sizeof(float));
	//计算每一像素的脊线方向值
	for (long y = SEMISIZ + 1; y < IHeight - SEMISIZ - 1; y++) //逐行遍历（除去边缘）
	{
		for (long x = SEMISIZ + 1; x < IWidth - SEMISIZ - 1; x++) //逐列遍历（除去边缘）
		{
			//计算以当前像素为中心的领域窗口区内每一像素的梯度
			for (long j = 0; j < SEMISIZ * 2 + 1; j++)
			{
				for (long i = 0; i < SEMISIZ * 2 + 1; i++)
				{
					int index1 = (y + j - SEMISIZ) * IWidth + x + i - SEMISIZ;
					int index2 = (y + j - SEMISIZ) * IWidth + x + i - SEMISIZ - 1;
					int index3 = (y + j - SEMISIZ - 1) * IWidth + x + i - SEMISIZ;
					dx[i][j] = long(ucImg[index1] - ucImg[index2]);
					dy[i][j] = long(ucImg[index1] - ucImg[index3]);
				}
			}
			//计算当前像素的脊线方向值
			fx = 0.0;
			fy = 0.0;
			for (long j = 0; j < SEMISIZ * 2 + 1; j++)
			{
				for (long i = 0; i < SEMISIZ * 2 + 1; i++)
				{
					fx += 2 * dx[i][j] * dy[i][j];
					fy += (dx[i][j] * dx[i][j] - dy[i][j] * dy[i][j]);

				}
			}
			fDirc[y * IWidth + x] = atan2(fx, fy);
		}

	}
	return 0;
}

//P98：脊线方向低通滤波 fDirc(脊线方向原始数据),fFitDirc(脊线方向滤波结果数据)。
int DircLowPass(float* fDirc, float* fFitDirc, long lWidth, long lHeight)
{
	//定义变量
	const int DIR_FILTER_SIZE = 2;
	long blocksize = 2 * DIR_FILTER_SIZE + 1;
	long imgsize = lWidth * lHeight;

	float* filter = new float[blocksize * blocksize];//使用5*5滤波
	float* phix = new float[imgsize];
	float* phiy = new float[imgsize];
	float* phi2x = new float[imgsize];
	float* phi2y = new float[imgsize];

	//结果初始化
	memset(fFitDirc, 0, sizeof(float) * lWidth * lHeight);

	//设置5*5搞斯低通滤波器模板
	float tempSum = 0.0;
	for (long y = 0; y < blocksize; y++)
	{
		for (long x = 0; x < blocksize; x++)
		{
			filter[y * blocksize + x] = (float)(blocksize - (abs(DIR_FILTER_SIZE - x) + abs(DIR_FILTER_SIZE - y)));
			tempSum += filter[y * blocksize + x];
		}
	}

	for (long y = 0; y < blocksize; y++)
	{
		for (long x = 0; x < blocksize; x++)
		{
			filter[y * blocksize + x] /= tempSum;
		}
	}

	//计算各像素点的方向正弦值和余弦值
	for (long y = 0; y < lHeight; y++)
	{
		for (long x = 0; x < lWidth; x++)
		{
			phix[y * lWidth + x] = cos(fDirc[y * lWidth + x]);
			phiy[y * lWidth + x] = sin(fDirc[y * lWidth + x]);
		}
	}
	//对所有像素进行方向低通滤波
	memset(phi2x, 0, sizeof(float) * imgsize);
	memset(phi2y, 0, sizeof(float) * imgsize);

	float nx, ny;
	long val;

	for (long y = 0; y < lHeight - blocksize; y++)//逐行遍历（除去边缘区段）
	{
		for (long x = 0; x < lWidth - blocksize; x++)//逐列遍历（除去边缘区段）
		{
			//对当前像素为中心的滤波窗口内的所有像素进行加权累加
			nx = 0.0;
			ny = 0.0;

			for (long j = 0; j < blocksize; j++)
			{
				for (long i = 0; i < blocksize; i++)
				{
					val = (x + i) + (j + y) * lWidth;
					nx += filter[j * blocksize + i] * phix[val];//方向余弦加权累加
					ny += filter[j * blocksize + i] * phiy[val];//方向正弦加权累加

				}
			}
			//将累加结果作为当前像素的新的方向正弦值和余弦值
			val = x + y * lWidth;
			phi2x[val] = nx;
			phi2y[val] = ny;

		}
	}

	//根据加权累加结果，计算各像素的方向滤波结果值
	for (long y = 0; y < lHeight - blocksize; y++)
	{
		for (long x = 0; x < lWidth - blocksize; x++)
		{
			val = x + y * lWidth;
			fFitDirc[val] = atan2(phi2y[val], phi2x[val]) * 0.5;
		}
	}

	delete[]phi2y;
	delete[]phi2x;
	delete[]phiy;
	delete[]phix;

	return 0;
}

//P107：指纹脊线频率计算      源图数据       脊线方向数据       脊线频率结果数据    图像宽         高
int Frequency(unsigned char* ucImg, float* fDirection, float* fFrequency, long lWidth, long lHeight) {
	//窗口大小
	const int SIZE_L = 32;
	const int SIZE_W = 16;
	const int SIZE_L2 = 16;
	const int SIZE_W2 = 8;

	//正弦波峰值点
	int peak_pos[SIZE_L];
	int peak_cnt;
	float peak_freq;
	float Xsig[SIZE_L];

	//方向
	float dir = 0.0;
	float cosdir = 0.0;
	float sindir = 0.0;
	float maxPeak, minPeak;

	//结果初始化
	float* frequency1 = new float[lWidth * lHeight];
	memset(fFrequency, 0, sizeof(float) * lWidth * lHeight);
	memset(frequency1, 0, sizeof(float) * lWidth * lHeight);

	long x, y;
	long d, k;
	long u, v;

	for (y = SIZE_L2; y < lHeight - SIZE_L2; y++) {//逐行遍历（除去边缘地区）
		for (x = SIZE_L2; x < lWidth - SIZE_L2; x++) {//逐列遍历（除去边缘地区）
			//当前像素的脊线方向
			dir = fDirection[(y + SIZE_W2) * lWidth + (x + SIZE_W2)];
			cosdir = -sin(dir);
			sindir = cos(dir);
			//计算以当前像素为中心的L*W邻域窗口的幅值序列，X[0]..X[L-1]
			for (k = 0; k < SIZE_L; k++) {
				Xsig[k] = 0.0;
				for (d = 0; d < SIZE_W; d++) {
					u = (long)(x + (d - SIZE_W2) * cosdir + (k - SIZE_L2) * sindir);
					v = (long)(y + (d - SIZE_W2) * sindir + (k - SIZE_L2) * cosdir);
					//边界点处理
					if (u < 0) {
						u = 0;
					}
					else if (u > lWidth - 1) {
						u = lWidth - 1;
					}
					if (v < 0) {
						v = 0;
					}
					else if (v > lHeight - 1) {
						v = lHeight - 1;
					}
					Xsig[k] += ucImg[u + v * lWidth];
				}
				Xsig[k] /= SIZE_W;
			}

			//确定幅值序列变化范围
			maxPeak = minPeak = Xsig[0];
			for (k = 0; k < SIZE_L; k++) {
				if (minPeak > Xsig[k]) {
					minPeak = Xsig[k];
				}
				if (maxPeak < Xsig[k]) {
					maxPeak = Xsig[k];
				}
			}

			//确定峰值点位置
			peak_cnt = 0;
			if ((maxPeak - minPeak) > 64) {
				for (k = 0; k < SIZE_L; k++) {
					if ((Xsig[k - 1] < Xsig[k]) && (Xsig[k] >= Xsig[k + 1])) {
						peak_pos[peak_cnt++] = k;
					}
				}
			}

			//计算峰值点平均间距
			peak_freq = 0.0;
			if (peak_cnt >= 2) {
				for (k = 0; k < peak_cnt - 1; k++) {
					peak_freq += (peak_pos[k + 1] - peak_pos[k]);
				}
				peak_freq /= peak_cnt - 1;

			}

			//计算当前像素的频率
			if (peak_freq < 3.0 || peak_freq>25.0) {
				frequency1[x + y * lWidth] = 0.0;
			}
			else {
				frequency1[x + y * lWidth] = 1.0 / peak_freq;
			}
		}
	}

	//对频率尽享均值滤波
	for (y = SIZE_L2; y < lHeight - SIZE_L2; y++) {//逐行遍历（除去边缘地区）
		for (x = SIZE_L2; x < lWidth - SIZE_L2; x++) {//逐列遍历（除去边缘地区）
			k = x + y * lWidth;//当前像素位置（在频率数组中的数组下标）
			peak_freq = 0.0;
			//使用以当前像素为中心的5*5邻域窗口进行均值滤波
			for (v = -2; v <= 2; v++) {
				for (u = -2; u <= 2; u++) {
					peak_freq += frequency1[(x + u) + (y + v) * lWidth];//求频率累加和
				}
			}
			fFrequency[k] = peak_freq / 25;//当前像素频率等于窗口内频率均值
		}
	}
	delete frequency1;
	return 0;
}

//P116：指纹掩码计算    
//ucImg(源数据),fDirection(脊线方向数据),fFrequency(脊线频率数据),ucMask(掩码结果数据),lWidth(图像宽带),lHeight(图像高度)
int GetMask(unsigned char* ucImg, float* fDirection,float* fFrequency, unsigned char* ucMask, long lWidth, long lHeight)
{
	//第1步：阈值分割（图像频率位于指定范围之内则设为前景点，否则设为背景点）
	float freqMin = 1.0 / 25.0;
	float freqMax = 1.0 / 3.0;
	long x, y, k;
	long  pos, posout;

	memset(ucMask, 0, lWidth * lHeight);//结果初始化（全为背景点）

	for (y = 0; y < lHeight; y++)//逐行遍历
	{
		for (x = 0; x < lWidth; x++)
		{
			pos = x + y * lWidth;
			posout = x + y * lWidth;
			ucMask[posout] = 0;
			if (fFrequency[pos] >= freqMin && fFrequency[pos] <= freqMax)
			{
				ucMask[posout] = 255;//频率位于指定范围则设为前景点
			}
		}
	}
	//第2步：填充空洞(利用形态学的膨胀原理)
	for (k = 0; k < 4; k++)
	{
		//标记前景点
		for (y = 1; y < lHeight - 1; y++)//逐行遍历
		{
			for (x = 1; x < lWidth - 1; x++)//主列遍历
			{
				//前景点的上下左右四个相邻点都标记为前景点（将8位像素的最高位设为1予以标记）
				if (ucMask[x + y * lWidth] == 0xFF)//前景点
				{
					ucMask[x - 1 + y * lWidth] = 0x80;
					ucMask[x + 1 + y * lWidth] = 0x80;
					ucMask[x + (y - 1) * lWidth] = 0x80;
					ucMask[x + (y + 1) * lWidth] = 0x80;
				}
			}
		}
		//判断和设置前景点
		for (y = 1; y < lHeight - 1; y++)
		{
			for (x = 1; x < lWidth - 1; x++)
			{
				//将标记为前景点的像素都设为前景点
				if (ucMask[x + y * lWidth])
				{
					ucMask[x + y * lWidth] = 0xFF;//设为前景点（像素值为255）
				}
			}
		}
	}
	//第3步：去除边缘和孤立点（利用形态学的腐蚀原理）
	for (k = 0; k < 12; k++)//重复腐蚀多次（次数自定）
	{
		//标记前景点
		for (y = 1; y < lHeight - 1; y++)//逐行遍历
		{
			for (x = 1; x < lWidth - 1; x++)//主列遍历
			{
				//背景点的上下左右四个相邻点都标记为潜在背景点（将8位像素值的后7都位设为0予以标记）
				if (ucMask[x + y * lWidth] == 0x0)//背景点
				{
					ucMask[x - 1 + y * lWidth] = 0x80;
					ucMask[x + 1 + y * lWidth] = 0x80;
					ucMask[x + (y - 1) * lWidth] = 0x80;
					ucMask[x + (y + 1) * lWidth] = 0x80;
				}
			}
		}
		//判断和设置前景点
		for (y = 1; y < lHeight - 1; y++)
		{
			for (x = 1; x < lWidth - 1; x++)
			{
				//只要不是前景点的所有像素都设为背景点
				if (ucMask[x + y * lWidth] != 0xFF)//非前景点
				{
					ucMask[x + y * lWidth] = 0x0;//设为前景点（像素值为0）
				}
			}
		}
	}
	return 0;
}



//P125: Gabor滤波增强 			源图数据       脊线方向数据       脊线频率数据			      掩码数据			      滤波增强结果数据      图像宽度   高度 						
int GaborEnhance(unsigned char* ucImg, float* fDirection, float* fFrequency, unsigned char* ucMask, unsigned char* ucImgEnhanced, long lWidth, long lHeight) {
	//定义变量
	const float PI = 3.141592654;
	long i, j, u, v;
	int wg2 = 5;//使用11*11的Gabor 滤波器，半边长为5
	float sum, f, g;
	float x2, y2;
	float dx2 = 1.0 / (4.0 * 4.0);//沿x轴方向的高斯包络常量
	float dy2 = 1.0 / (4.0 * 4.0);//沿y轴方向的高斯包络常量

	//结果初始化
	memset(ucImgEnhanced, 0, lWidth * lHeight);

	//Gabor滤波
	for (j = wg2; j < lHeight - wg2; j++) {//逐行遍历（除去边缘区段）
		for (i = wg2; i < lWidth - wg2; i++) {//逐列遍历（除去边缘区段）
											//跳过背景点
			if (ucMask[i + j * lWidth] == 0) {//掩码为0表示背景点
				continue;
			}
			//获取当前像素的方向和频率
			g = fDirection[i + j * lWidth];//当前像素的脊线方向
			f = fFrequency[i + j * lWidth];//当前像素的脊线频率
			g += PI / 2;//垂直方向

			//对当前像素进行Gabor滤波
			sum = 0.0;
			for (v = -wg2; v <= wg2; v++) {
				for (u = -wg2; u <= wg2; u++) {
					x2 = -u * sin(g) + v * cos(g);//x坐标旋转
					y2 = u * cos(g) + v * sin(g);//y坐标旋转
					sum += exp(-0.5 * (x2 * x2 * dx2 + y2 * y2 * dy2)) * cos(2 * PI * x2 * f) * ucImg[(i - u) + (j - v) * lWidth];//窗口内滤波值累加
				}
			}
			//边界值处理
			if (sum > 255.0) {
				sum = 255.0;
			}
			else if (sum < 0.0) {
				sum = 0.0;
			}
			//得到当前像素的滤波结果
			ucImgEnhanced[i + j * lWidth] = (unsigned char)sum;
		}
	}
	return 0;

}
//掩码计算，二值化，细化，特征匹配
int BinaryImg(unsigned char* ucImg, unsigned char* ucBinImg, long lWidth, long lHeight, unsigned char uThreshold) {
	unsigned char* pStart = ucImg;
	unsigned char* pEnd = ucImg + lWidth * lHeight;
	unsigned char* pDest = ucBinImg;
	while (pStart < pEnd) {
		*pDest = *pStart > uThreshold ? 1 : 0;
		pStart++;
		pDest++;
	}
	return 0;
}
int BinaryToGray(unsigned char* ucBinImg, unsigned char* ucGrayImg, long lWidth, long lHeight) {
	unsigned char* pStart = ucBinImg;
	unsigned char* pEnd = ucBinImg + lWidth * lHeight;
	unsigned char* pDest = ucGrayImg;
	while (pStart < pEnd) {
		*pDest = *pStart > 0 ? 0 : 255;
		pStart++;
		pDest++;
	}
	return 0;
}
// P144：图像细化             源图数据                    结果图数据            图像宽       高度          最大迭代次数
int Thinning(unsigned char* ucBinedImg, unsigned char* ucThinnedImage, long lWidth, long lHeight, long lIterativeLimit) {
	//定义变量
	unsigned char x1, x2, x3, x4, x5, x6, x7, x8, xp;
	unsigned char g1, g2, g3, g4;
	unsigned char b1, b2, b3, b4;
	unsigned char np1, np2, npm;
	unsigned char* pUp, * pDown, * pImg;
	long IDeletedPoints = 0;

	//结果初始化
	memcpy(ucThinnedImage, ucBinedImg, lWidth * lHeight);

	for (long it = 0; it < lIterativeLimit; it++) {//重复执行，一直到无点可删为止
		IDeletedPoints = 0;//初始化本次迭代所删点数

		//本次迭代内第一次遍历（使用条件G1&G2&G3）
		for (long i = 1; i < lHeight - 1; i++) {//逐行遍历
			//初始化邻域指针
			pUp = ucBinedImg + (i - 1) * lWidth;
			pImg = ucBinedImg + i * lWidth;
			pDown = ucBinedImg + (i + 1) * lWidth;

			for (long j = 1; j < lWidth - 1; j++) {//逐列遍历
				//调整邻域指针
				pUp++;
				pImg++;//当前像素（窗口中心像素）
				pDown++;
				//跳过背景点（背景点像素值为0）
				if (!*pImg) {
					continue;
				}

				//获取3*3邻域窗口内所有9个像素的灰度值
				x6 = *(pUp - 1);
				x5 = *(pImg - 1);
				x4 = *(pDown - 1);

				x7 = *pUp;
				xp = *pImg;
				x3 = *pDown;

				x8 = *(pUp + 1);
				x1 = *(pImg + 1);
				x2 = *(pDown + 1);

				//判断条件G1
				b1 = !x1 && (x2 == 1 || x3 == 1);
				b2 = !x3 && (x4 == 1 || x5 == 1);
				b3 = !x5 && (x6 == 1 || x7 == 1);
				b4 = !x7 && (x8 == 1 || x1 == 1);

				g1 = (b1 + b2 + b3 + b4) == 1;

				//判断条件G2
				np1 = x1 || x2;
				np1 += x3 || x4;
				np1 += x5 || x6;
				np1 += x7 || x8;
				np2 = x2 || x3;
				np2 += x4 || x5;
				np2 += x6 || x7;
				np2 += x8 || x1;

				npm = np1 > np2 ? np2 : np1;
				g2 = npm >= 2 && npm <= 3;

				//判断条件G3和G4
				g3 = (x1 && (x2 || x3 || !x8)) == 0;
				g4 = (x5 && (x6 || x7 || !x4)) == 0;


				//组合判断（使用条件G1 & G2 & G4）
				if (g1 && g2 && g4) {
					ucThinnedImage[lWidth * i + j] = 0;
					++IDeletedPoints;
				}
			}
		}
		//结果同步
		memcpy(ucBinedImg, ucThinnedImage, lWidth * lHeight);
		//本次迭代内第2次遍历（使用条件 G1 & G2 & G3）
		for (long i = 1; i < lHeight - 1; i++) {
			//初始化邻域指针
			pUp = ucBinedImg + (i - 1) * lWidth;
			pImg = ucBinedImg + i * lWidth;
			pDown = ucBinedImg + (i + 1) * lWidth;

			for (long j = 1; j < lWidth - 1; j++) {//逐列遍历
				//调整邻域指针
				pUp++;
				pImg++;//当前像素（窗口中心像素）
				pDown++;
				//跳过背景点（背景点像素值为0）
				if (!*pImg) {
					continue;
				}

				//获取3*3邻域窗口内所有9个像素的灰度值
				x6 = *(pUp - 1);
				x5 = *(pImg - 1);
				x4 = *(pDown - 1);

				x7 = *pUp;
				xp = *pImg;
				x3 = *pDown;

				x8 = *(pUp + 1);
				x1 = *(pImg + 1);
				x2 = *(pDown + 1);

				//判断条件G1
				b1 = !x1 && (x2 == 1 || x3 == 1);
				b2 = !x3 && (x4 == 1 || x5 == 1);
				b3 = !x5 && (x6 == 1 || x7 == 1);
				b4 = !x7 && (x8 == 1 || x1 == 1);

				g1 = (b1 + b2 + b3 + b4) == 1;

				//判断条件G2
				np1 = x1 || x2;
				np1 += x3 || x4;
				np1 += x5 || x6;
				np1 += x7 || x8;
				np2 = x2 || x3;
				np2 += x4 || x5;
				np2 += x6 || x7;
				np2 += x8 || x1;

				npm = np1 > np2 ? np2 : np1;
				g2 = npm >= 2 && npm <= 3;

				//判断条件G3和G4
				g3 = (x1 && (x2 || x3 || !x8)) == 0;
				g4 = (x5 && (x6 || x7 || !x4)) == 0;


				//组合判断（使用条件G1 & G2 & G4）
				if (g1 && g2 && g3) {
					ucThinnedImage[lWidth * i + j] = 0;
					++IDeletedPoints;
				}
			}
		}
		//结果同步
		memcpy(ucBinedImg, ucThinnedImage, lWidth * lHeight);

		//如果本次迭代已无点可删，则停止迭代
		if (IDeletedPoints == 0) {
			break;
		}
	}

	//清除边缘区段
	for (long i = 0; i < lHeight; i++) {
		for (long j = 0; j < lWidth; j++) {
			if (i < 16) {
				ucThinnedImage[i * lWidth + j] = 0;
			}
			else if (i >= lHeight - 16) {
				ucThinnedImage[i * lWidth + j] = 0;
			}
			else if (j < 16) {
				ucThinnedImage[i * lWidth + j] = 0;
			}
			else if (j >= lWidth - 16) {
				ucThinnedImage[i * lWidth + j] = 0;
			}
		}
	}
	return 0;
}

//P156：指纹特征提取  
long Extract(unsigned char* ucThinImg, unsigned char* ucMinuImg, long lWidth, long lHeight) {
	//定义变量
	unsigned char* pDest = ucMinuImg;//结果图数据指针
	unsigned char* pUp, * pDown, * pImg;//源图邻域指针
	unsigned char x1, x2, x3, x4, x5, x6, x7, x8;//八邻点
	unsigned char nc;//八邻点中黑点数量
	long lMinuCount = 0;//特征点数量

						//结果初始化（全设为0，表示都是非特征点）
	memset(pDest, 0, lWidth * lHeight);

	//遍历源图以提取指纹特征
	for (long i = 1; i < lHeight - 1; i++) {//逐行遍历
										  //初始化邻域指针
		pUp = ucThinImg + (i - 1) * lWidth;
		pDown = ucThinImg + i * lWidth;
		pImg = ucThinImg + (i + 1) * lWidth;

		for (long j = 1; j < lWidth - 1; j++) {//逐列遍历
											 //调整邻域指针
			pUp++;
			pImg++;
			pDown++;


			//跳过背景点（背景点像素值为0）
			if (!*pImg) {
				continue;
			}

			//获取3*3邻域窗口内所有八个邻点像素的灰度值
			x6 = *(pUp - 1);
			x5 = *(pImg - 1);
			x4 = *(pDown - 1);

			x7 = *pUp;
			x3 = *pDown;


			x8 = *(pUp + 1);
			x1 = *(pImg + 1);
			x2 = *(pDown + 1);

			//统计八邻点中黑点数量
			nc = x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;

			//特征点判断
			if (nc == 1) {//端点
				pDest[i * lWidth + j] = 1;//结果图中对应像素点设为1（表示端点）
				++lMinuCount;//特征点数量加1
			}
			else if (nc == 3) {//分叉点
				pDest[i * lWidth + j] = 3;//结果图中对应像素点设为3（表示分叉点）
				++lMinuCount;//特征点数量加1
			}

		}
	}


	return lMinuCount;//返回特征点数量
}

// P168：去除边缘特征点 
int CutEdge(MINUTIAE * minutiaes, long count, unsigned char* ucImg, long lWidth, long lHeight)
{
	//定义变量
	long minuCount = count;
	long x, y, type;
	bool del;

	//初始化标记数组
	long* pFlag = new long[minuCount];//标记数组（标记值0-保存，1-删除）
	memset(pFlag, 0, sizeof(long) * minuCount);//初始化（全0全保留）
	//遍历所有特征点
	for (long i = 0; i < minuCount; i++)
	{
		//获取当前特征点信息
		y = minutiaes[i].y - 1;//纵坐标（行坐标）
		x = minutiaes[i].x - 1;//横坐标（列坐标）
		type = minutiaes[i].type;//特征点类型（1-端点，3-分叉点）

		//将当前特征点的删除标记初始化为true
		del = true;

		//根据当前特征点的位置判断其是否边缘特征点
		if (x < lWidth / 2)//如果位于图像左半图
		{
			if (abs(lWidth / 2 - x) > abs(lHeight / 2 - y))//如果位于图像左半图的左侧
			{
				//在特征图中查找当前特征点同一行左侧是否还有其他特征点
				while (--x >= 0)//逐一左移查找
				{
					//如果在左侧存在其他特征点，则说明当前特征点不是边缘特征点，就无须删除
					if (ucImg[x + y * lWidth] > 0)//特征图像素值（0-非特征点，1-端点，3-分叉点）
					{
						del = false;//删除标记位置为false
						break;//停止当前特征点的左移查找过程
					}
				}
			}
			else//如果位于图像左半图的右侧
			{
				if (y > lHeight / 2)//如果位于图像左半图的右下侧
				{
					//在特征图中查找当前特征点同一列下侧是否还有其他特征点
					while (++y < lHeight)//逐一下移查找
					{
						//如果在下侧存在其他特征点，则说明当前特征点不是边缘特征点，就无须删除
						if (ucImg[x + y * lWidth] > 0)//特征图像素值（0-非特征点，1-端点，3-分叉点）
						{
							del = false;//删除标记位置为false
							break;//停止当前特征点的下移查找过程
						}
					}
				}
				else//如果位于图像左半图的右上侧
				{
					//在特征图中查找当前特征点同一列上侧是否还有其他特征点
					while (--y >= 0)//逐一上移查找
					{
						//如果在上侧存在其他特征点，则说明当前特征点不是边缘特征点，就无须删除
						if (ucImg[x + y * lWidth] > 0)//特征图像素值（0-非特征点，1-端点，3-分叉点）
						{
							del = false;//删除标记位置为false
							break;//停止当前特征点的上移查找过程
						}
					}
				}
			}
		}
		else//如果位于图右半图
		{
			if (abs(lWidth / 2 - x) > abs(lHeight / 2 - y))//如果位于图像右半图的右侧
			{
				//在特征图中查找当前特征点同一行右侧是否还有其他特征点
				while (++x < lWidth)//逐一右移查找
				{
					//如果在右侧存在其他特征点，则说明当前特征点不是边缘特征点，就无须删除
					if (ucImg[x + y * lWidth] > 0)//特征图像素值（0-非特征点，1-端点，3-分叉点）
					{
						del = false;//删除标记位置为false
						break;//停止当前特征点的右移查找过程
					}
				}
			}
			else//如果位于图像右半图的左侧
			{
				if (y > lHeight / 2)//如果位于图像右半图的左下侧
				{
					//在特征图中查找当前特征点同一列下侧是否还有其他特征点
					while (++y < lHeight)//逐一下移查找
					{
						//如果在下侧存在其他特征点，则说明当前特征点不是边缘特征点，就无须删除
						if (ucImg[x + y * lWidth] > 0)//特征图像素值（0-非特征点，1-端点，3-分叉点）
						{
							del = false;//删除标记位置为false
							break;//停止当前特征点的下移查找过程
						}
					}
				}
				else//如果位于图像右半图的左上侧
				{
					//在特征图中查找当前特征点同一列上侧是否还有其他特征点
					while (--y >= 0)//逐一上移查找
					{
						//如果在上侧存在其他特征点，则说明当前特征点不是边缘特征点，就无须删除
						if (ucImg[x + y * lWidth] > 0)//特征图像素值（0-非特征点，1-端点，3-分叉点）
						{
							del = false;//删除标记位置为false
							break;//停止当前特征点的上移查找过程
						}
					}
				}
			}
		}

		//如果当前特征点是边缘特征点，则予以删除（在标记数组中将其标记设置为1）
		if (del)//如果当前特征点的删除标记为true，则表明其为边缘特征点，应予以删除
		{
			pFlag[i] = 1;//在标记数组中将其标记设置为1（表示删除）
			continue;//继续判断结构数组中的下一个特征点
		}
	}

	//重组特征点结构数组（在当前结构数组中将所有有效特征点前移）
	long newCount = 0;//有效特征点数量（同时也是重组后的有效特征点数组下标）
	for (long i = 0; i < minuCount; i++)
	{
		if (pFlag[i] == 0)//如果当前特征点需要保留（标记值：0-保留，1-删除）
		{
			memcpy(&minutiaes[newCount], &minutiaes[i], sizeof(MINUTIAE));//特征结构点结构整体复制
			newCount++;//有效特征点下标后移（有效特征点数量加1）
		}
	}

	delete pFlag;
	pFlag = NULL;
	//返回有效特征点数量
	return newCount;
}

//P163：特征过滤   
//minuData(特征图数据)，thinData(细化数据)，minutiaes(特征点数组)，minuCount(特征点数量)
int MinuFilter(unsigned char* minuData, unsigned char* thinData, MINUTIAE* minutiaes, long
	& minuCount, long lWidth, long lHeight)
{
	//第1步：计算细化图中各点方向
	float* dir = new float[lWidth * lHeight];
	memset(dir, 0, lWidth * lHeight * sizeof(float));

	ImgDirection(thinData, dir, lWidth, lHeight);//计算脊线方向

	//第2步：从特征图中提取特征点数据
	unsigned char* pImg;
	unsigned char val;
	int temp = 0;
	for (long i = 1; i < lHeight - 1; i++)//逐行遍历
	{
		pImg = minuData + i * lWidth;
		for (long j = 1; j < lWidth - 1; j++)//逐列遍历
		{
			//获取特征图数据
			++pImg;//特征图指针后移
			val = *pImg;//特征图像素值（0-非特征点，1-端点，3-分叉点）
			if (val > 0)
			{
				minutiaes[temp].x = j + 1;//横坐标（从1开始）
				minutiaes[temp].y = i + 1;//纵坐标（从1开始）
				minutiaes[temp].theta = dir[i * lWidth + j];//脊线方向
				minutiaes[temp].type = long(val);//特征点类型（1-端点，3-分叉点）
				++temp;//特征点数组指针后移	
			}
		}
	}
	delete[] dir;

	//第3步：去除边缘特征点
	minuCount = CutEdge(minutiaes, minuCount, thinData, lWidth, lHeight);

	//第4步：去除毛刺、小孔、间断等伪特征点

	//初始化标记数组
	long* pFlag = new long[minuCount];//标记数组（标记值：0-保存，1-删除）
	memset(pFlag, 0, sizeof(long) * minuCount);

	//遍历所有特征点
	long x1, x2, y1, y2, type1, type2;
	for (long i = 0; i < minuCount; i++)
	{
		//获取特征点1的信息
		x1 = minutiaes[i].x;
		y1 = minutiaes[i].y;
		type1 = minutiaes[i].type;//特征点类型（1-端点，3-分叉点）
		for (long j = i + 1; j < minuCount; j++)
		{
			//跳过已删特征点
			if (pFlag[j] == 1)
			{
				continue;
			}
			//获取特征点2的信息
			x2 = minutiaes[j].x;
			y2 = minutiaes[j].y;
			type2 = minutiaes[j].type;//特征点类型（1-端点，3-分叉点）

			//计算两点间距
			long r = (long)sqrt(float((y1 - y2) * (y1 - y2) + (x1 - x2) * (x1 - x2)));

			//删除间距过小的特征点
			if (r <= 4)//如果间距不大于4则认为间距过小（间距阈值可自定）
			{
				if (type1 == type2)//如果两点类型相同
				{
					if (type1 == 1)//如果两点都是端点，则认定为“短线或纹线间断”
					{
						pFlag[i] = pFlag[j] = 1;//同时删除掉两点
					}
					else//如果两点都是分叉点，则认定为小孔
					{
						pFlag[j] = 1;//只删除掉点2

					}
				}
				else if (type1 == 1)//如果点1是端点而点2是分叉点，则点1为“毛刺”
				{
					pFlag[i] = 1;//只删除掉点1
				}
				else
				{
					pFlag[j] = 1;//只删除掉点2
				}
			}
		}
	}

	//重组特征点结构数组（在当前结构数组中将所有有效特征点前移）
	long newCount = 0;//有效特征点数量（同时也是重组后的有效特征点数组下标）
	for (long i = 0; i < minuCount; i++)
	{
		if (pFlag[i] == 0)
		{
			memcpy(&minutiaes[newCount], &minutiaes[i], sizeof(MINUTIAE));//特征点结构 整体复制

			newCount++;//有效特征点下标后移（有效特征点数量加1）
		}
	}
	delete[]pFlag;
	minuCount = newCount;//保存有效特征点数量
	//返回结果
	return 0;
}

//P179：保存特征模板文件     minutiaes(特征点数组)，count(特征点数量)，fileName(模板文件名)
int SaveMinutiae(MINUTIAE* minutiaes, long count, char* fileName)
{
	//打开文件（二进制写入方式）
	FILE* fp = fopen(fileName, "wb");
	if (!fp)
	{
		return -1;
	}

	//将所有特征点的结构体数据整体写入文件
	const static long TemplateFlag = 0x3571027f;
	fwrite(&TemplateFlag, sizeof(long), 1, fp);//写入特征模板文件标记
	fwrite(&count, sizeof(long), 1, fp);//写入特征点数量

	for (long i = 0; i < count; i++)
	{
		fwrite(&(minutiaes[i]), sizeof(MINUTIAE), 1, fp);//将特征点的结构整体写入
	}
	//关闭文件
	fclose(fp);
	return 0;
}

//P182：获得新指纹编号 
int GetNewIndexDB()
{
	//从index文件中读取最后一条记录的编号
	int sNo = 0;
	char name[MAX_PATH] = { 0 }, srcFile[MAX_PATH] = { 0 }, mdlFile[MAX_PATH] = { 0 };

	FILE* index = fopen(DB_INDEX_TXT, "r");
	while (!feof(index))
	{
		fscanf(index, "%d %s %s %s\n", &sNo, srcFile, mdlFile, name);
	}
	fclose(index);

	//生成新编号
	sNo = sNo + 1;
	//返回新编号
	return sNo;
}

//P186：宽字符串转换为多字节字符串 
char* ToChar(wchar_t* str)
{
	int num = WideCharToMultiByte(0, 0, str, -1, NULL, 0, NULL, false);
	char* chStr = new char[num];
	WideCharToMultiByte(0, 0, str, -1, chStr, num, NULL, false);

	return chStr;
}

//P194：计算线段倾斜度 
float Angle2Points(long x1, long y1, long x2, long y2) {
	const float PI = 3.141592654;
	float diffY, diffX;
	float theta = 0.0;

	diffY = y2 - y1;
	diffX = x2 - x1;

	if (diffY < 0 && diffX>0) {
		theta = atan2(-1 * diffY, diffX);
	}
	else if (diffY < 0 && diffX < 0) {
		theta = PI - atan2(-1 * diffY, -1 * diffX);
	}
	else if (diffY > 0 && diffX < 0) {
		theta = atan2(diffY, -1 * diffX);
	}
	else if (diffY > 0 && diffX > 0) {
		theta = PI - atan2(diffY, diffX);
	}
	else if (diffX == 0) {
		theta = PI / 2;
	}
	else {
		theta = 0.0;
	}
	return theta;
}

//P191：构建特征点相邻关系 
int BuildNabors(MINUTIAE* minutiae, long minuCount) {


	//定义变量
	const int MAX_NEIGHBOR_EACH = 10;//每个特征点最多保存10个相邻特征点
	long x1, x2, y1, y2;
	long* pFlag = new long[minuCount];//相邻标记数组（标记值：0-不相邻，1-相邻）

									  //遍历特征点数组
	for (long i = 0; i < minuCount; i++) {
		//获取当前特征点信息
		x1 = minutiae[i].x;
		y1 = minutiae[i].y;

		//初始化当前特征点的相邻标记数组
		memset(pFlag, 0, sizeof(long) * minuCount);//初始化为全0（不相邻）
		pFlag[i] = 1;//将自身标记为“相邻”


					 //为当前特征点创建并初始化相邻特征点结构数组
		minutiae[i].neibors = new NEIGHBOR[MAX_NEIGHBOR_EACH];//创建相邻特征点结构数组
		if (minutiae[i].neibors == NULL) {
			return -1;
		}
		memset(minutiae[i].neibors, 0, sizeof(NEIGHBOR) * MAX_NEIGHBOR_EACH);//初始化数组

																		   //查找和保存10个相邻特征点
		for (long neighborNo = 0; neighborNo < MAX_NEIGHBOR_EACH; neighborNo++) {//重复10次

																			   //初始化最小间隔和对应特征点下标
			long minDistance = 1000;//最小间隔
			long minNo = 0;//最小间距对应的特征点下标

						   //查找相邻特征点之外的最近的不相邻特征点
			for (long j = 0; j < minuCount; j++) { //每次都遍历所有特征点
												 //跳过已找到的相邻特征点
				if (pFlag[j] == 1) {//(标记值：0-不相邻，1-相邻)
					continue;
				}

				//获取特征点2的信息
				x2 = minutiae[j].x;
				y2 = minutiae[j].y;

				//计算两点间距
				long r = (long)sqrt(float((y1 - y2) * (y1 - y2) + (x1 - x2) * (x1 - x2)));

				//查找最小间距
				if (r < minDistance) {
					minNo = j;
					minDistance = r;

				}

			}

			//保存查找结果
			pFlag[minNo] = 1;
			minutiae[i].neibors[neighborNo].x = minutiae[minNo].x;
			minutiae[i].neibors[neighborNo].y = minutiae[minNo].y;
			minutiae[i].neibors[neighborNo].type = minutiae[minNo].type;
			minutiae[i].neibors[neighborNo].Theta = Angle2Points(minutiae[minNo].x, minutiae[minNo].y, x1, y1);
			minutiae[i].neibors[neighborNo].Theta2Ridge = minutiae[minNo].theta - minutiae[i].theta;
			minutiae[i].neibors[neighborNo].ThetaThisNibor = minutiae[minNo].theta;
			minutiae[i].neibors[neighborNo].distance = minDistance;
		}
	}
	delete[] pFlag;
	return 0;
}

//P195：特征匹配相似度算法  
float MinuSimilarity(MINUTIAE* minutiae1, long count1, MINUTIAE* minutiae2, long count2) {

	const int MAX_SIMILAR_PAIR = 100;//最多保存100对配对相似特征点
	const int MAX_NEIGHBOR_EACH = 10;//每个特征点最多保存10个相邻特征点

	BuildNabors(minutiae1, count1);
	BuildNabors(minutiae2, count2);

	long similarPair[MAX_SIMILAR_PAIR][2];
	memset(similarPair, 0, 100 * 2 * sizeof(long));

	MINUTIAE* baseMinutiae;
	MINUTIAE* refMinutiae;

	long baseAccount, refAccount;
	if (count1 < count2) {
		baseMinutiae = minutiae1;
		baseAccount = count1;
		refMinutiae = minutiae2;
		refAccount = count2;
	}
	else {
		baseMinutiae = minutiae2;
		baseAccount = count2;
		refMinutiae = minutiae1;
		refAccount = count1;
	}

	NEIGHBOR* baseNeighbors = NULL;
	NEIGHBOR* refNeighbors = NULL;
	long similarMinutiae = 0;
	float baseTheta, refTheta;

	for (long i = 0; i < baseAccount; i++) {
		baseNeighbors = baseMinutiae[i].neibors;
		baseTheta = baseMinutiae[i].theta;

		long refSimilarNo = 0;
		long maxSimilarNeibors = 0;
		for (long j = 0; j < refAccount; j++) {
			if (refMinutiae[j].type != baseMinutiae[i].type) {
				continue;
			}

			refNeighbors = refMinutiae[j].neibors;
			refTheta = refMinutiae[j].theta;

			//统计相似相邻点数量
			long thisSimilarNeigbors = 0;
			for (long m = 0; m < MAX_NEIGHBOR_EACH; m++) {
				for (long n = 0; n < MAX_NEIGHBOR_EACH; n++) {
					//跳过类型不同的相邻点
					if (baseNeighbors[m].type != refNeighbors[n].type) {
						continue;
					}
					long dist = abs(long(baseNeighbors[m].distance - refNeighbors[n].distance));
					float theta1 = abs(float((baseNeighbors[m].Theta - baseTheta) - (refNeighbors[n].Theta - refTheta)));
					float theta2 = abs(float(baseNeighbors[m].Theta2Ridge - refNeighbors[n].Theta2Ridge));
					float theta3 = abs(float((baseNeighbors[m].Theta - baseNeighbors[m].ThetaThisNibor) - (refNeighbors[n].Theta - refNeighbors[n].ThetaThisNibor)));

					if (dist < 3.9 && theta1 < 0.148f && theta2 < 0.147f && theta3 < 0.147f) {
						++thisSimilarNeigbors;
						break;
					}
				}
			}
			//如果3对以上相邻点相似，则认为当前基准点与当前参考点相似，保存匹配结果。
			if ((thisSimilarNeigbors >= MAX_NEIGHBOR_EACH * 5 / 10) && (similarMinutiae < MAX_SIMILAR_PAIR)) {
				similarPair[similarMinutiae][0] = i;
				similarPair[similarMinutiae][1] = refSimilarNo;
				++similarMinutiae;
			}
		}
	}
	//计算特征匹配相似度
	float similarity = similarMinutiae / 8.0f;
	similarity = similarMinutiae < 2 ? 0.0f : similarity;
	similarity = similarMinutiae > 8 ? 1.0f : similarity;

	return similarity;
}

//P200：读取特征模板文件 fileName(特征模板文件名)，minutiae(特征点数组)
int ReadMinutiae(char* fileName, MINUTIAE** minutiae)
{
	//打开文件（二进制读取方式）
	FILE* fp = fopen(fileName, "rb");
	if (!fp) {
		return -1;
	}
	//逐一整体读取所有特征点的结构体数据
	const static long TemplateFileFlag = 0x3571027f;
	long flag;
	fread(&flag, sizeof(long), 1, fp);//读取特征模板文件标记
	if (flag != TemplateFileFlag)
	{
		return -2;
	}

	long account;
	fread(&account, sizeof(long), 1, fp);//读取特征点数量
	*minutiae = new MINUTIAE[account];//创建特征点结构数组
	if (*minutiae == NULL)
	{
		return -3;
	}

	for (long i = 0; i < account; i++) {
		fread(&((*minutiae)[i]), sizeof(MINUTIAE), 1, fp);//整体读取特征点结构
	}

	//关闭文件
	fclose(fp);
	return account;
}

//P206：指纹库预检（判断指纹库是否为空）  
bool EmptyDB()
{
	//打开文件
	char indexFile[MAX_PATH] = { DB_INDEX_TXT };//指纹库引索文件
	FILE* pFile = fopen(indexFile, "r");
	if (!pFile)//若引索文件不存在或存在异常，则指纹库为空
	{
		return true;
	}

	//判断指纹库是否为空（引索文件内容是否为空）
	int size = filelength(fileno(pFile));//文件长度（以字节为单位）

	bool bEmpty = (size < 1) ? true : false;//若索引文件内容为空，则指纹库为空

	//关闭文件
	fclose(pFile);
	//返回判断结果
	return bEmpty;
}

//P212：指纹登记过程集成
//regName(登记人姓名),srcImgFile(原图文件名),outImgFile(特征结果图文件名),info(返回操作或失败提示信息),bSaveImg(是否保存所有中间结果图像)
int Enroll(char* regName, char* srcImgFile, char* outImgFile, char* info, bool bSaveImg)
{
	//第0步：设置中间结果文件名
	//中间结果数据文件名
	char stepTxtFile1[MAX_PATH] = { STEP_TXT_1 };
	char stepTxtFile2[MAX_PATH] = { STEP_TXT_2 };
	char stepTxtFile3[MAX_PATH] = { STEP_TXT_3 };
	char stepTxtFile4[MAX_PATH] = { STEP_TXT_4 };
	char stepTxtFile5[MAX_PATH] = { STEP_TXT_5 };
	char stepTxtFile6[MAX_PATH] = { STEP_TXT_6 };
	char stepTxtFile7[MAX_PATH] = { STEP_TXT_7 };
	char stepTxtFile8[MAX_PATH] = { STEP_TXT_8 };
	char stepTxtFile9[MAX_PATH] = { STEP_TXT_9 };
	char stepTxtFile10[MAX_PATH] = { STEP_TXT_10 };
	char stepTxtFile11[MAX_PATH] = { STEP_TXT_11 };

	//中间结果图像文件名
	char stepImgFile1[MAX_PATH] = { STEP_IMG_1 };
	char stepImgFile2[MAX_PATH] = { STEP_IMG_2 };
	char stepImgFile3[MAX_PATH] = { STEP_IMG_3 };
	char stepImgFile4[MAX_PATH] = { STEP_IMG_4 };
	char stepImgFile5[MAX_PATH] = { STEP_IMG_5 };
	char stepImgFile6[MAX_PATH] = { STEP_IMG_6 };
	char stepImgFile7[MAX_PATH] = { STEP_IMG_7 };
	char stepImgFile8[MAX_PATH] = { STEP_IMG_8 };
	char stepImgFile9[MAX_PATH] = { STEP_IMG_9 };
	char stepImgFile10[MAX_PATH] = { STEP_IMG_10 };
	char stepImgFile11[MAX_PATH] = { STEP_IMG_11 };
	char stepImgFile12[MAX_PATH] = { STEP_IMG_12 };

	//指纹特征模板名
	char stepMdlFile11[MAX_PATH] = { STEP_IMG_11_MDL };

	//第1步：图像载入，读取源图数据和参数信息
	long lWidth, lHeight, lDepth;
	ReadBMPImgFilePara(srcImgFile, lWidth, lHeight, lDepth);
	unsigned char* image1 = new unsigned char[lWidth * lHeight];
	unsigned char* image2 = new unsigned char[lWidth * lHeight];
	ReadBMPImgFileDataq(srcImgFile, image1);//读取源图数据
	memset(image2, 0, lWidth * lHeight);//结果图初始化
	if (bSaveImg)
	{
		CopyFile(ToWideChar(srcImgFile), ToWideChar(stepImgFile1), false);
		SaveDataToTextFile(stepTxtFile1, image1, lWidth, lHeight);

	}

	//第2步：中值滤波
	MidFilter(image1, image2, lWidth, lHeight);//中值滤波
	if (bSaveImg)
	{
		SaveDataToImageFile(srcImgFile, stepImgFile2, image2);
		SaveDataToTextFile(stepTxtFile2, image2, lWidth, lHeight);
	}
	//第3步：直方图均衡化
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);//以第2步滤波结果为源图数据
	HistoNormalize(image1, image2, lWidth, lHeight);//直方图均衡化
	if (bSaveImg)
	{
		SaveDataToImageFile(srcImgFile, stepImgFile3, image2);
		SaveDataToTextFile(stepTxtFile3, image2, lWidth, lHeight);
	}
	//第4步：方向计算
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);//以第3步均衡化结果为源图数据
	float* tmpDirection = new float[lWidth * lHeight];
	float* direction = new float[lWidth * lHeight];
	ImgDirection(image1, tmpDirection, lWidth, lHeight);//方向计算
	DircLowPass(tmpDirection, direction, lWidth, lHeight);//方向低通滤波
	if (bSaveImg)
	{
		const int DIRECTION_SCALE = 100;//方向结果转换比例（仅用于结果显示）
		SaveDataToImageFile(srcImgFile, stepImgFile4, direction, DIRECTION_SCALE);
		SaveDataToTextFile(stepTxtFile4, direction, lWidth, lHeight);
	}
	delete[] tmpDirection;

	//第5步：频率计算
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);//以第3步均衡化结果为源图数据
	float* frequency = new float[lWidth * lHeight];
	Frequency(image1, direction, frequency, lWidth, lHeight);//频率计算
	if (bSaveImg)
	{
		const int FREQUENCY_SCALE = 1000;//频率结果转换比例（仅用于结果显示）
		SaveDataToImageFile(srcImgFile, stepImgFile5, frequency, FREQUENCY_SCALE);
		SaveDataToTextFile(stepTxtFile5, frequency, lWidth, lHeight);
	}


	//第6步：掩码计算
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);//以第3步均衡化结果为源图数据
	unsigned char* mask = new unsigned char[lWidth * lHeight];
	GetMask(image1, direction, frequency, mask, lWidth, lHeight); //掩码计算
	if (bSaveImg)
	{
		SaveDataToImageFile(srcImgFile, stepImgFile6, mask);
		SaveDataToTextFile(stepTxtFile6, mask, lWidth, lHeight);
	}

	//第7步：Gabor滤波增强
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);//以第3步均衡化结果为源图数据
	GaborEnhance(image1, direction, frequency, mask, image2, lWidth, lHeight); //增强
	if (bSaveImg)
	{
		SaveDataToImageFile(srcImgFile, stepImgFile7, image2);
		SaveDataToTextFile(stepTxtFile7, image2, lWidth, lHeight);
	}
	delete[]direction;
	delete[]frequency;
	delete[]mask;

	//第8步：二值化
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);//以第7步Gabor滤波结果为源图数据
	BinaryImg(image1, image2, lWidth, lHeight, 128); //二值化
	if (bSaveImg)
	{
		SaveDataToImageFile(srcImgFile, stepImgFile8, image2);
		SaveDataToTextFile(stepTxtFile8, image2, lWidth, lHeight);
	}

	//第9步：细化
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);//以第8步二值化结果为源图数据
	Thinning(image1, image2, lWidth, lHeight, 128); //细化
	if (bSaveImg)
	{
		SaveDataToImageFile(srcImgFile, stepImgFile9, image2);
		SaveDataToTextFile(stepTxtFile9, image2, lWidth, lHeight);
	}
	unsigned char* thin = new unsigned char[lWidth * lHeight];
	memcpy(thin, image2, sizeof(unsigned char) * lWidth * lHeight);//保存细化结果


	//第10步：特征提取
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);//以第9步细化结果为源图数据
	long minuCount = Extract(image1, image2, lWidth, lHeight); //特征提取
	if (bSaveImg)
	{
		SaveDataToImageFile(srcImgFile, stepImgFile10, image2);
		SaveDataToTextFile(stepTxtFile10, image2, lWidth, lHeight);
	}

	//第11步：特征过滤
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);//以第10步特征提取结果为源图数据
	MINUTIAE* minutiaes = new MINUTIAE[minuCount];
	memset(minutiaes, sizeof(MINUTIAE), minuCount);
	MinuFilter(image1, thin, minutiaes, minuCount, lWidth, lHeight);//特征过滤


	/*将特征过滤结果保存到outImageFile*/
	unsigned char* image3 = new unsigned char[lWidth * lHeight];
	memset(image3, 0, lWidth * lHeight);
	for (long i = 0; i < minuCount; i++)
	{
		image3[(minutiaes[i].y - 1) * lWidth + (minutiaes[i].x - 1)] = 0xff;
	}

	SaveDataToImageFile(srcImgFile, outImgFile, image3);
	
	delete[]thin;

	SaveMinutiae(minutiaes, minuCount, stepMdlFile11);//保存特征数据文件

	if (bSaveImg)
	{
		memset(image2, 0, lWidth * lHeight);
		for (long i = 0; i < minuCount; i++)
		{
			image2[(minutiaes[i].y - 1) * lWidth + (minutiaes[i].x - 1)] = 0xff;
		}
		SaveDataToImageFile(srcImgFile, stepImgFile11, image2);
		SaveDataToTextFile(stepTxtFile11, image2, lWidth, lHeight);

	}

	//第12步：特征入库
	int sNo = GetNewIndexDB();//获得数据库新指纹记录编号
	char dbImgFile[MAX_PATH] = { 0 };//数据库中源图文件名
	char dbMdlFile[MAX_PATH] = { 0 };//数据库中模板文件名
	sprintf(dbImgFile, "%s%d.bmp", DB_DIR, sNo);//设置源图文件名（存入数据看文件夹内），以编号命名
	sprintf(dbMdlFile, "%s%d.mdl", DB_DIR, sNo);//设置模板文件名（存入数据看文件夹内），以编号命名

	if (minuCount > 0)
	{
		//保存原始指纹图像
		CopyFile(ToWideChar(srcImgFile), ToWideChar(dbImgFile), false);
		//保存指纹库特征模板文件
		CopyFile(ToWideChar(stepMdlFile11), ToWideChar(dbMdlFile), false);

		//指纹登记信息写入数据库引索文件
		FILE* index = fopen(DB_INDEX_TXT, "a");//追加模式写入
		fprintf(index, "%d %s %s %s\n", sNo, dbImgFile, dbMdlFile, regName);
		fclose(index);
	}
	delete[]image1;
	delete[]image2;
	delete[]minutiaes;
	delete[]image3;

	sprintf(info, "指纹登记成功。");

	return 0;
}

//P225：指纹识别过程集成 
int Identify(char* srcImgFile, char* outImgFile, char* info, bool bSaveImg) {
	//第0步：设置中间结果文件名
	//中间结果数据文件名
	char stepTxtFile1[MAX_PATH] = { STEP_TXT_1 };
	char stepTxtFile2[MAX_PATH] = { STEP_TXT_2 };
	char stepTxtFile3[MAX_PATH] = { STEP_TXT_3 };
	char stepTxtFile4[MAX_PATH] = { STEP_TXT_4 };
	char stepTxtFile5[MAX_PATH] = { STEP_TXT_5 };
	char stepTxtFile6[MAX_PATH] = { STEP_TXT_6 };
	char stepTxtFile7[MAX_PATH] = { STEP_TXT_7 };
	char stepTxtFile8[MAX_PATH] = { STEP_TXT_8 };
	char stepTxtFile9[MAX_PATH] = { STEP_TXT_9 };
	char stepTxtFile10[MAX_PATH] = { STEP_TXT_10 };
	char stepTxtFile11[MAX_PATH] = { STEP_TXT_11 };

	//中间结果图像文件名
	char stepImgFile1[MAX_PATH] = { STEP_IMG_1 };
	char stepImgFile2[MAX_PATH] = { STEP_IMG_2 };
	char stepImgFile3[MAX_PATH] = { STEP_IMG_3 };
	char stepImgFile4[MAX_PATH] = { STEP_IMG_4 };
	char stepImgFile5[MAX_PATH] = { STEP_IMG_5 };
	char stepImgFile6[MAX_PATH] = { STEP_IMG_6 };
	char stepImgFile7[MAX_PATH] = { STEP_IMG_7 };
	char stepImgFile8[MAX_PATH] = { STEP_IMG_8 };
	char stepImgFile9[MAX_PATH] = { STEP_IMG_9 };
	char stepImgFile10[MAX_PATH] = { STEP_IMG_10 };
	char stepImgFile11[MAX_PATH] = { STEP_IMG_11 };
	char stepImgFile12[MAX_PATH] = { STEP_IMG_12 };

	//指纹特征模板文件名
	char stepMdlFile11[MAX_PATH] = { STEP_IMG_11_MDL };

	//指纹库预检（判断指纹库是否为空）
	bool j = EmptyDB();
	if (j) {
		sprintf(info, "指纹库为空");
		return 0;
	}

	//第1步：图像载入，读取源图数据和参数信息
	long lWidth, lHeight, lDepth;
	ReadBMPImgFilePara(srcImgFile, lWidth, lHeight, lDepth);
	unsigned char* image1 = new unsigned char[lWidth * lHeight];
	unsigned char* image2 = new unsigned char[lWidth * lHeight];
	ReadBMPImgFileDataq(srcImgFile, image1);
	memset(image2, 0, lWidth * lHeight);
	if (bSaveImg) {
		CopyFile(ToWideChar(srcImgFile), ToWideChar(stepImgFile1), false);
		SaveDataToTextFile(stepTxtFile1, image1, lWidth, lHeight);
	}
	//第2步：中值滤波
	MidFilter(image1, image2, lWidth, lHeight);
	if (bSaveImg) {
		SaveDataToImageFile(srcImgFile, stepImgFile2, image2);
		SaveDataToTextFile(stepTxtFile2, image2, lWidth, lHeight);
	}
	//第3步：直方图均衡化
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);
	HistoNormalize(image1, image2, lWidth, lHeight);
	if (bSaveImg) {
		SaveDataToImageFile(srcImgFile, stepImgFile3, image2);
		SaveDataToTextFile(stepTxtFile3, image2, lWidth, lHeight);
	}
	//第4步：方向计算
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);

	float* tmpDirection = new float[lWidth * lHeight];
	float* direction = new float[lWidth * lHeight];
	ImgDirection(image1, tmpDirection, lWidth, lHeight);
	DircLowPass(tmpDirection, direction, lWidth, lHeight);
	if (bSaveImg) {
		const int DIRECTION_SCALE = 100;
		SaveDataToImageFile(srcImgFile, stepImgFile4, direction, DIRECTION_SCALE);
		SaveDataToTextFile(stepTxtFile4, direction, lWidth, lHeight);
	}
	delete[] tmpDirection;

	//第5步：频率计算
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);
	float* frequency = new float[lWidth * lHeight];
	Frequency(image1, direction, frequency, lWidth, lHeight);
	if (bSaveImg) {
		const int FREQUENCY_SCALE = 1000;
		SaveDataToImageFile(srcImgFile, stepImgFile5, frequency, FREQUENCY_SCALE);
		SaveDataToTextFile(stepTxtFile5, frequency, lWidth, lHeight);
	}

	//第6步：掩码计算
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);
	unsigned char* mask = new unsigned char[lWidth * lHeight];
	GetMask(image1, direction, frequency, mask, lWidth, lHeight);
	if (bSaveImg) {
		SaveDataToImageFile(srcImgFile, stepImgFile6, mask);
		SaveDataToTextFile(stepTxtFile6, mask, lWidth, lHeight);
	}

	//第7步：Gabor滤波增强
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);
	GaborEnhance(image1, direction, frequency, mask, image2, lWidth, lHeight);
	if (bSaveImg) {
		SaveDataToImageFile(srcImgFile, stepImgFile7, image2);
		SaveDataToTextFile(stepTxtFile7, image2, lWidth, lHeight);
	}
	delete[] direction;
	delete[] frequency;
	delete[] mask;

	//第8步：二值化
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);
	BinaryImg(image1, image2, lWidth, lHeight, 128);
	if (bSaveImg) {
		SaveDataToImageFile(srcImgFile, stepImgFile8, image2);
		SaveDataToTextFile(stepTxtFile8, image2, lWidth, lHeight);
	}
	//第9步：细化
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);
	Thinning(image1, image2, lWidth, lHeight, 128);
	if (bSaveImg) {
		SaveDataToImageFile(srcImgFile, stepImgFile9, image2);
		SaveDataToTextFile(stepTxtFile9, image2, lWidth, lHeight);
	}
	unsigned char* thin = new unsigned char[lWidth * lHeight];
	memcpy(thin, image2, sizeof(unsigned char) * lWidth * lHeight);

	//第10步：特征提取
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);
	long minuCount = Extract(image1, image2, lWidth, lHeight);
	if (bSaveImg) {
		SaveDataToImageFile(srcImgFile, stepImgFile10, image2);
		SaveDataToTextFile(stepTxtFile10, image2, lWidth, lHeight);
	}

	//第11步：特征过滤
	memcpy(image1, image2, sizeof(unsigned char) * lWidth * lHeight);
	MINUTIAE* minutiaes = new MINUTIAE[minuCount];
	memset(minutiaes, sizeof(MINUTIAE), minuCount);
	MinuFilter(image1, thin, minutiaes, minuCount, lWidth, lHeight);
	delete[] thin;
	SaveMinutiae(minutiaes, minuCount, stepMdlFile11);
	if (bSaveImg) {
		memset(image2, 0, lWidth * lHeight);
		for (long i = 0; i < minuCount; i++) {
			image2[(minutiaes[i].y - 1) * lWidth + (minutiaes[i].x - 1)] = 0xff;
		}
		SaveDataToImageFile(srcImgFile, stepImgFile11, image2);
		SaveDataToTextFile(stepTxtFile11, image2, lWidth, lHeight);
	}
	delete[] image1;
	delete[] image2;


	//第12步：特征识别
	int id = 0;
	char name[MAX_PATH] = { 0 }, src[MAX_PATH] = { 0 }, mdl[MAX_PATH] = { 0 };
	float maxSimilar = -1;
	int maxID = -1;
	char maxName[MAX_PATH] = { 0 }, maxSrc[MAX_PATH] = { 0 };
	MINUTIAE* minutiaes2 = NULL;
	long minuCount2 = 0;

	FILE* index = fopen(DB_INDEX_TXT, "r");

	while (!feof(index)) {
		fscanf(index, "%d %s %s %s\n", &id, src, mdl, name);

		//从特征模板文件中读取指纹特征
		minuCount2 = ReadMinutiae(mdl, &minutiaes2);

		//特征匹配
		float similar = MinuSimilarity(minutiaes, minuCount, minutiaes2, minuCount2);

		//保存相似度最高的指纹记录信息
		if (similar > maxSimilar) {
			maxSimilar = similar;
			maxID = id;
			sprintf(maxName, name);
			sprintf(maxSrc, src);
		}
	}

	fclose(index);
	delete[] minutiaes;
	delete[] minutiaes2;

	//返回识别结果
	const float SIMILAR_THRED = 0.1;
	if (maxID < 0 || maxSimilar < SIMILAR_THRED) {
		sprintf(info, "识别失败");
		return -12;
	}

	strcpy(outImgFile, maxSrc);

	if (bSaveImg) {
		CopyFile(ToWideChar(maxSrc), ToWideChar(stepImgFile12), false);
	}
	sprintf(info, "识别成功.识别结果：姓名[%s],目标指纹[%s],相似度[%.2f]", maxName, maxSrc, maxSimilar);
	return 0;
}

//登记按钮触发事件
void CFingerDlg::OnBnClickedBtnAddImage()
{
	// TODO: 在此添加控件通知处理程序代码
	//初始化操作结果信息
	char info[MAX_PATH] = { 0 };

	//获取界面输入内容（登记人姓名）
	UpdateData(true);
	if (m_name.GetLength() == 0)
	{
		MessageBox(_T("请输入姓名！"), _T("提示"));//显示信息
		return;
	}

	if (m_name1.GetLength() == 0)
	{
		MessageBox(_T("请输入密码！"), _T("提示"));//显示信息
		return;
	}

	if (m_name2.GetLength() == 0)
	{
		MessageBox(_T("请输入确认密码！"), _T("提示"));//显示信息
		return;
	}
	
	//选择图像
	//改进点：可以自由选择要打开的文件；
	CString filter = L"bmp文件(*.bmp)|*.bmp||";
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter);
	char srcImgFile[MAX_PATH] = { 0 };
	if (dlg.DoModal() == IDOK)//判断是否按下"保存"按钮
	{

		CString str = dlg.GetPathName();

		//将宽字符串转换成多字节字符串
		int num = WideCharToMultiByte(0, 0, str, -1, NULL, 0, NULL, false);
		char* chStr = new char[num];
		WideCharToMultiByte(0, 0, str, -1, chStr, num, NULL, false);

		strcpy(srcImgFile, chStr);
	}
	
	//指纹登记
	bool bSaveImg = false;//是否保存中间结果图像

	char outImgFile[MAX_PATH] = { OUTIMAGE };//指纹特征结果图像文件名

	Enroll(ToChar(m_name.GetBuffer()), srcImgFile, outImgFile, info, bSaveImg);//指纹登记
	m_staticInfo.SetWindowText(ToWideChar(info));

	//显示图像
	ShowImageInCtrl(m_picCtrl1, srcImgFile);//显示原始指纹图像；
	ShowImageInCtrl(m_picCtrl2, outImgFile);//显示指纹特征图像；
	UpdateData(false);
}

//识别按钮触发事件
void CFingerDlg::OnBnClickedBtnMatchimage()
{
	// TODO: 在此添加控件通知处理程序代码
	char info[MAX_PATH] = { 0 };
	m_staticInfo.SetWindowText(ToWideChar(""));

	/*char srcImgFile[MAX_PATH] = "capt.bmp";*/
	//改进点：可以自由选择要打开的文件；
	CString filter = L"bmp文件(*.bmp)|*.bmp||";
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter);
	char srcImgFile[MAX_PATH] = { 0 };
	if (dlg.DoModal() == IDOK)//判断是否按下"保存"按钮
	{

		CString str = dlg.GetPathName();

		//将宽字符串转换成多字节字符串
		int num = WideCharToMultiByte(0, 0, str, -1, NULL, 0, NULL, false);
		char* chStr = new char[num];
		WideCharToMultiByte(0, 0, str, -1, chStr, num, NULL, false);

		strcpy(srcImgFile, chStr);
	}

	bool bSaveImg = false;
	char outImgFile[MAX_PATH] = { 0 };
	int i = Identify(srcImgFile, outImgFile, info, bSaveImg);
	m_staticInfo.SetWindowText(ToWideChar(info));

	ShowImageInCtrl(m_picCtrl1, srcImgFile);
	if (i == 0)//如果识别成功返回匹配到的指纹图像
	{
		//  SqlConnection conn = new SqlConnection("Data Source=DESKTOP-M56GOV6\SQLEXPRESS;Initial Catalog=HRM;Integrated Security=True");
		ShowImageInCtrl(m_picCtrl2, outImgFile);
		
		UpdateData(TRUE);//更新变量信息
		InfoFile file;
		CStringA tmp;
		CStringA tmp2;
		if (m_name1 != m_name2)
		{
			MessageBox(_T("两次输入的密码不一致!"));
		}
		else
		{
			tmp = m_name;
			tmp2 = m_name1;
			file.writeRegister(tmp.GetBuffer(), tmp2.GetBuffer());//将char类型转化为CString,并进行用户名和密码的写入
			MessageBox(_T("注册成功!"));
		}
	}
	else {
		MessageBox(_T("识别失败！"), _T("提示"));
	}
}

void CFingerDlg::OnBnClickedBtnRegister()
{
	// TODO: 在此添加控件通知处理程序代码
	//初始化操作结果信息
	char info[MAX_PATH] = { 0 };

	//获取界面输入内容（登记人姓名）
	UpdateData(true);
	if (m_name.GetLength() == 0)
	{
		MessageBox(_T("请输入姓名！"), _T("提示"));//显示信息
		return;
	}
	if (m_name1.GetLength() == 0)
	{
		MessageBox(_T("请输入密码！"), _T("提示"));//显示信息
		return;
	}

	if (m_name2.GetLength() == 0)
	{
		MessageBox(_T("请输入确认密码！"), _T("提示"));//显示信息
		return;
	}

	//实时采集图像
	char srcImgFile[MAX_PATH] = "capt.bmp";
	m_zkfpEng.SaveBitmap(ToWideChar(srcImgFile));

	//指纹登记
	bool bSaveImg = false;//是否保存中间结果图像
	char outImgFile[MAX_PATH] = { OUTIMAGE };//指纹特征结果图像文件名
	Enroll(ToChar(m_name.GetBuffer()), srcImgFile, outImgFile, info, bSaveImg);//指纹登记
	m_staticInfo.SetWindowText(ToWideChar(info));

	//显示图像
	ShowImageInCtrl(m_picCtrl1, srcImgFile);//显示原始指纹图像；
	ShowImageInCtrl(m_picCtrl2, outImgFile);//显示指纹特征图像
}


void CFingerDlg::OnBnClickedBtnIdentify()
{
	// TODO: 在此添加控件通知处理程序代码
	char info[MAX_PATH] = { 0 };
	m_staticInfo.SetWindowText(ToWideChar(""));

	//实时采集
	char srcImgFile[MAX_PATH] = "capt.bmp";

	m_zkfpEng.SaveBitmap(ToWideChar(srcImgFile));


	bool bSaveImg = false;

	char outImgFile[MAX_PATH] = { 0 };

	int num = Identify(srcImgFile, outImgFile, info, bSaveImg);

	m_staticInfo.SetWindowText(ToWideChar(info));

	ShowImageInCtrl(m_picCtrl1, srcImgFile);

	if (num == 0)//如果识别成功返回匹配到的指纹图像
	{
		ShowImageInCtrl(m_picCtrl2, outImgFile);

		InfoFile file;
		CStringA tmp;
		CStringA tmp2;

		if (m_name1 != m_name2)
		{
			MessageBox(_T("两次输入的密码不一致!"));
		}
		else
		{
			tmp = m_name;
			tmp2 = m_name1;
			file.writeRegister(tmp.GetBuffer(), tmp2.GetBuffer());//将char类型转化为CString,并进行用户名和密码的写入
			MessageBox(_T("注册成功!"));
		}
	}
	else {
		MessageBox(_T("识别失败！"), _T("提示"));
	}
}

