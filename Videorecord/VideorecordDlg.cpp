
// VideorecordDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "Videorecord.h"
#include "VideorecordDlg.h"
#include "afxdialogex.h"

#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/core/core.hpp>+
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <Mil.h>
#include <string>
#include <mutex>
#include <filesystem>
#include <chrono>

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#define GRAB_NUM 100

using namespace cv;
using namespace std;
using namespace chrono;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

MIL_INT TOTAL_FRAME_NUM;
MIL_ID MilApplication;
MIL_ID MilSystem;
MIL_ID MilDigitizer1;
MIL_ID MilGrabDisplay1;
MIL_ID MilDispImage1;

MIL_ID ResultDisplay;
MIL_ID ResultDisplaybuf;

MIL_INT SizeX1;
MIL_INT SizeY1;
MIL_INT SizeBit1;
MIL_INT SizeBand = 3;
MIL_DOUBLE FrameRate;

MIL_ID MilImage1[GRAB_NUM];

MIL_ID MilImageColor1;
MIL_INT HookType;
MIL_ID HookId;
void* HookDataPtr;
CString sDriveCAM = _T("C:");
CTime file_time;
CString strTime;
CString filePath;
// CV VALUE
Mat Frame;

VideoWriter writer;
// int fourcc = VideoWriter::fourcc('a', 'v', 'c', '1'); // DEFAULT VALUE
// int fourcc = VideoWriter::fourcc('m', 'p', '4', 'v'); // DEFAULT VALUE
int fourcc = VideoWriter::fourcc('M', 'J', 'P', 'G'); // DEFAULT VALUE

CString file_format = _T(".avi"); // DEFAULT VALUEW

vector<Mat> Frame_buffer;
vector<Mat> Frame_buffer2;
vector<Mat> LFrame_buffer;

steady_clock::time_point save_start, save_end, encode_start, encode_end;
steady_clock::time_point grab_start, grab_end;

float imgcnt;
bool Init_Control = false;

typedef struct
{
	MIL_ID MilDigitizer1;
	MIL_ID MilGrabDisplay1;
	MIL_ID* MilImage1;
	MIL_ID MilDispImage1;
	MIL_INT GrabDoneCAM1;
	MIL_INT ProcesseStop1;
	long    ProcessedImageCount1;
	MIL_INT EventCount1;
	MIL_INT NbEventsReceived1;
	MIL_DOUBLE TimeStamp1;
} HookDataStruct;
HookDataStruct UserHookData;

void ProcessWindowMessage();
MIL_INT MFTYPE ProcessingFunction1(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CVideorecordDlg 대화 상자

CVideorecordDlg::CVideorecordDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VIDEORECORD_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVideorecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_DISP, m_ctrl_disp);
	DDX_Control(pDX, IDC_EDIT_LOG, m_ctrl_log);
	DDX_Control(pDX, IDC_BUTTON_INIT, m_btn_init);
	DDX_Control(pDX, IDC_BUTTON_START, m_btn_start);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_btn_stop);
	DDX_Control(pDX, IDC_BUTTON_FS, m_btn_framesearch);
	DDX_Control(pDX, IDC_BUTTON_PS, m_btn_pathsetting);
	DDX_Control(pDX, IDC_BUTTON_ENCODE, m_btn_encode);
	DDX_Control(pDX, IDC_STATIC_PATH, m_ctrl_folder);
	DDX_Control(pDX, IDC_EDIT_FRAME, m_edit_frame);
	DDX_Control(pDX, IDC_STATIC_DISP2, m_ctrl_disp2);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_btn_savebuf);
	DDX_Control(pDX, IDC_PROGRESS_PROG, m_ctrl_progress);
}

BEGIN_MESSAGE_MAP(CVideorecordDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(IDC_BUTTON_FS, &CVideorecordDlg::OnBnClickedButtonFs)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_INIT, &CVideorecordDlg::OnBnClickedButtonInit)
	ON_BN_CLICKED(IDC_BUTTON_START, &CVideorecordDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CVideorecordDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_ENCODE, &CVideorecordDlg::OnBnClickedButtonEncode)
	ON_BN_CLICKED(IDC_BUTTON_PS, &CVideorecordDlg::OnBnClickedButtonPs)
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CVideorecordDlg::OnBnClickedButtonSave)
END_MESSAGE_MAP()


// CVideorecordDlg 메시지 처리기

BOOL CVideorecordDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	SetWindowText(_T("Video Process"));
	// CreateDirectory(_T("cache"), NULL);

	GetDlgItem(IDC_STATIC_DISP)->MoveWindow(14, 50, 316, 316); // Disp1
	GetDlgItem(IDC_BUTTON_INIT)->MoveWindow(404, 109, 178, 35); // init
	GetDlgItem(IDC_BUTTON_START)->MoveWindow(404, 158, 178, 35); // Start
	GetDlgItem(IDC_BUTTON_STOP)->MoveWindow(404, 207, 178, 35); // Stop
	GetDlgItem(IDC_BUTTON_ENCODE)->MoveWindow(368, 256, 117, 35); // Encode
	GetDlgItem(IDC_BUTTON_SAVE)->MoveWindow(503, 256, 117, 35); // Encode
	GetDlgItem(IDC_STATIC_PATH)->MoveWindow(368, 73, 211, 22); // Path
	GetDlgItem(IDC_EDIT_FRAME)->MoveWindow(368, 325, 211, 24); // F / S
	GetDlgItem(IDC_BUTTON_PS)->MoveWindow(582, 73, 51, 22); // P / S
	GetDlgItem(IDC_BUTTON_FS)->MoveWindow(582, 325, 51, 22); // F / S
	GetDlgItem(IDC_EDIT_LOG)->MoveWindow(14, 430, 318, 292); // Log
	GetDlgItem(IDC_STATIC_DISP2)->MoveWindow(348, 430, 292, 292); // Disp2
	GetDlgItem(IDC_PROGRESS_PROG)->MoveWindow(14, 372, 619, 20);

	m_ctrl_progress.SetRange(0, 100);

	m_btn_init.EnableWindow(false);
	m_btn_start.EnableWindow(false);
	m_btn_stop.EnableWindow(false);
	m_btn_encode.EnableWindow(false);
	m_btn_framesearch.EnableWindow(false);
	m_btn_pathsetting.EnableWindow(true);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CVideorecordDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CVideorecordDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);

		CDC MemDC;
		CBitmap bmp;

		CRect rct;
		this->GetClientRect(&rct);

		MemDC.CreateCompatibleDC(&dc);
		bmp.LoadBitmap(IDB_BTM_BACK);
		MemDC.SelectObject(&bmp);

		dc.BitBlt(0, 0, rct.Width(), rct.Height(), &MemDC, 0, 0, SRCCOPY);

		// CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CVideorecordDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ###ProcessingFunction1
MIL_INT MFTYPE ProcessingFunction1(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
{
	HookDataStruct* UserHookDataPtr = (HookDataStruct*)HookDataPtr;

	MIL_ID ModifiedBufferIdCAM1;

	unsigned char* pData = Frame.data;

	save_start = steady_clock::now();

	/* Retrieve the MIL_ID of the grabbed buffer. */
	MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferIdCAM1);
	
	MbufCopy(ModifiedBufferIdCAM1, UserHookDataPtr->MilDispImage1);
	
	ProcessWindowMessage();

	MbufGet2d(ModifiedBufferIdCAM1, 0, 0, SizeX1, SizeY1, (void*)Frame.data);
	
	Mat temp = Frame.clone();

	Frame_buffer.push_back(temp);
	
	ProcessWindowMessage();

	UserHookDataPtr->ProcessedImageCount1++;

	save_end = steady_clock::now();

	milliseconds savesec = duration_cast<milliseconds>(save_end - save_start);

	//cout << savesec.count() << " ms" << endl;

	return 0;
}


void CVideorecordDlg::OnBnClickedButtonInit()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	m_btn_init.EnableWindow(false);
	m_btn_start.EnableWindow(false);
	m_btn_stop.EnableWindow(false);
	m_btn_encode.EnableWindow(false);
	m_btn_framesearch.EnableWindow(false);
	m_btn_pathsetting.EnableWindow(false);
	m_btn_savebuf.EnableWindow(false);

	file_time = CTime::GetCurrentTime();
	strTime = file_time.Format(_T("%Y%m%d_%H%M%S"));
	CreateDirectory(filePath + strTime, NULL);

	if (Init_Control == false) // Alloc 되지 않은 상태
	{
		MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL, &MilDigitizer1, M_NULL);
		MdigInquire(MilDigitizer1, M_SIZE_X, &SizeX1);
		MdigInquire(MilDigitizer1, M_SIZE_Y, &SizeY1);
		MdigInquire(MilDigitizer1, M_SIZE_BIT, &SizeBit1);
		MdigInquire(MilDigitizer1, M_SIZE_BAND, &SizeBand);
		MdigInquire(MilDigitizer1, M_SELECTED_FRAME_RATE, &FrameRate);
		MdigInquireFeature(MilDigitizer1, M_FEATURE_VALUE, MIL_TEXT("AcquisitionFrameRate"), M_TYPE_DOUBLE, &FrameRate);;

		// MdigControlFeature(MilDigitizer1, M_FEATURE_VALUE, MIL_TEXT("TestPattern"), M_TYPE_STRING, MIL_TEXT("GreyDiagonalRampMoving"));

		MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilGrabDisplay1);
		MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &ResultDisplay);

		Frame.create(SizeY1, SizeX1, CV_8UC1); // 인코딩용 Main Frame

		for (int i = 0; i < GRAB_NUM; i++)
		{
			MbufAlloc2d(MilSystem, SizeX1, SizeY1, SizeBit1 + M_UNSIGNED, M_IMAGE + M_GRAB, MilImage1 + i);
			MbufClear(MilImage1[i], 0);
		}

		MbufAlloc2d(MilSystem, SizeX1, SizeY1, SizeBit1 + M_UNSIGNED, M_IMAGE + M_GRAB + M_DISP, &ResultDisplaybuf);
		MbufAlloc2d(MilSystem, SizeX1, SizeY1, SizeBit1 + M_UNSIGNED, M_IMAGE + M_GRAB + M_DISP, &MilDispImage1); // Mono CAM

		MbufClear(MilDispImage1, 0);
		MbufClear(ResultDisplaybuf, 0);

		// Grab Buffer Display
		MdispSelectWindow(MilGrabDisplay1, MilDispImage1, m_ctrl_disp.GetSafeHwnd());
		MdispControl(MilGrabDisplay1, M_FILL_DISPLAY, M_ENABLE); // Image를 Picture Box에 맞게 채우도록 함.

		MdispSelectWindow(ResultDisplay, ResultDisplaybuf, m_ctrl_disp2.GetSafeHwnd());
		MdispControl(ResultDisplay, M_FILL_DISPLAY, M_ENABLE); // Image를 Picture Box에 맞게 채우도록 함.

		MdigControl(MilDigitizer1, M_GRAB_TIMEOUT, M_INFINITE);

		Init_Control = true; // ** Control Value True

		UserHookData.MilDigitizer1 = MilDigitizer1;
		UserHookData.MilImage1 = MilImage1;
		UserHookData.MilDispImage1 = MilDispImage1;
		UserHookData.ProcessedImageCount1 = 0;

		m_ctrl_log.ReplaceSel(_T("MIL System Ready! \r\n"));
		m_ctrl_log.ReplaceSel(_T("--------------------------------------------------------------------------------------------------\r\n"));

		AfxMessageBox(_T("Initialize OK!"));
	}
	else if (Init_Control == true)
	{
		MbufFree(MilDispImage1);
		MbufFree(ResultDisplaybuf);
		for (int i = 0; i < GRAB_NUM; i++)
		{
			MbufFree(MilImage1[i]);
		}
		MdispFree(ResultDisplay);
		MdispFree(MilGrabDisplay1);
		MappFreeDefault(MilApplication, MilSystem, M_NULL, MilDigitizer1, M_NULL);

		Init_Control = false;

		MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL, &MilDigitizer1, M_NULL);
		MdigInquire(MilDigitizer1, M_SIZE_X, &SizeX1);
		MdigInquire(MilDigitizer1, M_SIZE_Y, &SizeY1);
		MdigInquire(MilDigitizer1, M_SIZE_BIT, &SizeBit1);
		MdigInquire(MilDigitizer1, M_SIZE_BAND, &SizeBand);
		MdigInquire(MilDigitizer1, M_SELECTED_FRAME_RATE, &FrameRate);
		MdigInquireFeature(MilDigitizer1, M_FEATURE_VALUE, MIL_TEXT("AcquisitionFrameRate"), M_TYPE_DOUBLE, &FrameRate);;
		// MdigControlFeature(MilDigitizer1, M_FEATURE_VALUE, MIL_TEXT("TestPattern"), M_TYPE_STRING, MIL_TEXT("GreyDiagonalRampMoving"));

		MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilGrabDisplay1);
		MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &ResultDisplay);

		Frame.create(SizeY1, SizeX1, CV_8UC1); // 인코딩용 Main Frame

		for (int i = 0; i < GRAB_NUM; i++)
		{
			MbufAlloc2d(MilSystem, SizeX1, SizeY1, SizeBit1 + M_UNSIGNED, M_IMAGE + M_GRAB, MilImage1 + i);
			MbufClear(MilImage1[i], 0);
		}

		MbufAlloc2d(MilSystem, SizeX1, SizeY1, SizeBit1 + M_UNSIGNED, M_IMAGE + M_GRAB + M_DISP, &ResultDisplaybuf);
		MbufAlloc2d(MilSystem, SizeX1, SizeY1, SizeBit1 + M_UNSIGNED, M_IMAGE + M_GRAB + M_DISP, &MilDispImage1); // Mono CAM

		MbufClear(MilDispImage1, 0);
		MbufClear(ResultDisplaybuf, 0);

		// Grab Buffer Display
		MdispSelectWindow(MilGrabDisplay1, MilDispImage1, m_ctrl_disp.GetSafeHwnd());
		MdispControl(MilGrabDisplay1, M_FILL_DISPLAY, M_ENABLE); // Image를 Picture Box에 맞게 채우도록 함.

		MdispSelectWindow(ResultDisplay, ResultDisplaybuf, m_ctrl_disp2.GetSafeHwnd());
		MdispControl(ResultDisplay, M_FILL_DISPLAY, M_ENABLE); // Image를 Picture Box에 맞게 채우도록 함.

		MdigControl(MilDigitizer1, M_GRAB_TIMEOUT, M_INFINITE);

		Frame_buffer.clear();

		Init_Control = true; // ** Control Value True

		UserHookData.MilDigitizer1 = MilDigitizer1;
		UserHookData.MilImage1 = MilImage1;
		UserHookData.MilDispImage1 = MilDispImage1;
		UserHookData.ProcessedImageCount1 = 0;

		m_ctrl_log.ReplaceSel(_T("MIL System Ready! \r\n"));
		m_ctrl_log.ReplaceSel(_T("--------------------------------------------------------------------------------------------------\r\n"));

		AfxMessageBox(_T("Re - Initialize OK!"));
	}

	m_btn_init.EnableWindow(true);
	m_btn_start.EnableWindow(true);
	m_btn_stop.EnableWindow(false);
	m_btn_encode.EnableWindow(false);
	m_btn_framesearch.EnableWindow(false);
	m_btn_pathsetting.EnableWindow(true);
	m_btn_savebuf.EnableWindow(false);
}

void CVideorecordDlg::OnBnClickedButtonStart() // Grab Start 그랩 해서 밀 버퍼에서 Mat 버퍼로 옮김 (Vector)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_btn_init.EnableWindow(false);
	m_btn_start.EnableWindow(false);
	m_btn_stop.EnableWindow(true);
	m_btn_encode.EnableWindow(false);
	m_btn_framesearch.EnableWindow(false);
	m_btn_pathsetting.EnableWindow(false);
	m_btn_savebuf.EnableWindow(false);

	UserHookData.ProcessedImageCount1 = 0;
	UserHookData.GrabDoneCAM1 = 0;

	grab_start = steady_clock::now();

	MdigProcess(MilDigitizer1, MilImage1, GRAB_NUM, M_START, M_ASYNCHRONOUS, ProcessingFunction1, &UserHookData);

	//cout << "START SIGNAL" << endl;

	while (1)
	{
		if (UserHookData.GrabDoneCAM1 == 1)
		{
			MdigProcess(MilDigitizer1, MilImage1, GRAB_NUM, M_STOP, M_DEFAULT, ProcessingFunction1, &UserHookData);
			//cout << "STOP SIGNAL" << endl;

			writer.release();

			break;
		}
		ProcessWindowMessage();
	}
}


// ###BT_STOP
void CVideorecordDlg::OnBnClickedButtonStop() //  Stop (기존 Vector 의 인덱스는 가지고 있어야함)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.	

	UserHookData.GrabDoneCAM1 = 1;

	m_ctrl_log.ReplaceSel(_T("Grab End! \r\n"));
	m_ctrl_log.ReplaceSel(_T("--------------------------------------------------------------------------------------------------\r\n"));
	// m_ctrl_log.ReplaceSel(_T("--------------------------------------------\r\n"));

	waitKey(1000);

	m_btn_init.EnableWindow(true);
	m_btn_start.EnableWindow(false);
	m_btn_stop.EnableWindow(false);
	m_btn_encode.EnableWindow(true);
	m_btn_framesearch.EnableWindow(false);
	m_btn_pathsetting.EnableWindow(true);
	m_btn_savebuf.EnableWindow(true);
}

void ProcessWindowMessage()
{
	MSG msg;
	
	while (::PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
	{
		::SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
	}
}

// ### Encode
void CVideorecordDlg::OnBnClickedButtonEncode()
{ 
	m_btn_savebuf.EnableWindow(false);


	Size sz2(SizeX1, SizeY1);

	CString path;
	Mat temp;
	Mat temp2;

	m_ctrl_log.ReplaceSel(_T("Data Loading... \r\n"));

	writer.open("testing.avi", fourcc, 10, sz2, 0);
	save_start = steady_clock::now();
	for (int i = 0; i < 10; i++) {
		temp2 = Frame_buffer[i];
		writer.write(temp2);
	}

	save_end = steady_clock::now();
	milliseconds savesec = duration_cast<milliseconds>(save_end - save_start);
	
	// cout << savesec.count() / 10 << " ms (avg)" << endl;
	
	writer.release();

	double tFrameRate = (1000 / (savesec.count() / 10));

	// cout << frametime << endl;

	if (FrameRate > tFrameRate)
	{
		FrameRate = (1000 / (savesec.count() / 10)) - 4.0;
	}

	CString result, result2, result3, result4;
	result.Format(_T("%f"), FrameRate);
	result2.Format(_T("%d"), Frame_buffer.size());
	result3.Format(_T("%d"), SizeX1);
	result4.Format(_T("%d"), SizeY1);

	m_ctrl_log.ReplaceSel(_T("Support FrameRate for this system is ") + result + _T("\r\n"));
	m_ctrl_log.ReplaceSel(_T("Total Number of Frame : ") + result2 + _T("\r\n"));
	m_ctrl_log.ReplaceSel(_T("Frame Size : [ X : ") + result3 + _T(" , Y : ") + result4 + _T(" ]\r\n"));
	m_ctrl_log.ReplaceSel(_T("--------------------------------------------------------------------------------------------------\r\n"));

	m_ctrl_log.ReplaceSel(_T("Encoding Start! \r\n"));
	m_ctrl_log.ReplaceSel(_T("--------------------------------------------------------------------------------------------------\r\n"));

	int time = (1000 / FrameRate);

	temp2.create(sz2.height, sz2.width, CV_8UC1); // 인코딩용 Main Frame

	writer.open(string(CT2CA(filePath + strTime + _T("\\") + strTime + file_format)), fourcc, FrameRate, sz2, 0);

	imgcnt = Frame_buffer.size();

	int progress = imgcnt / 100;

	for (int i = 0; i < imgcnt; i++)
	{
		save_start = steady_clock::now();

		temp2 = Frame_buffer[i];
		writer.write(temp2);

		save_end = steady_clock::now();
		milliseconds savesec = duration_cast<milliseconds>(save_end - save_start);
		
		if ((i % progress) == 0)
		{
			m_ctrl_progress.OffsetPos(1);
		}
		//cout << "Remain : " << Frame_buffer.size() - i << " / Time : " << savesec.count() << " ms" << endl;
	}

	writer.release();

	m_ctrl_progress.OffsetPos(-100);
	m_ctrl_log.ReplaceSel(_T("Encoding End ! \r\n"));
	m_ctrl_log.ReplaceSel(_T("--------------------------------------------------------------------------------------------------\r\n"));

	m_btn_init.EnableWindow(true);
	m_btn_start.EnableWindow(false);
	m_btn_stop.EnableWindow(false);
	m_btn_encode.EnableWindow(false);
	m_btn_framesearch.EnableWindow(true);
	m_btn_pathsetting.EnableWindow(true);
	m_btn_savebuf.EnableWindow(true);
}

void CVideorecordDlg::OnBnClickedButtonPs()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString strInitPath = _T("C:\\");
	CString strDailyFolderPath;
	CString strOriFolderPath;

	// 폴더 선택 다이얼로그
	CFolderPickerDialog Picker(strInitPath, OFN_FILEMUSTEXIST, NULL, 0);
	if (Picker.DoModal() == IDOK)
	{
		strDailyFolderPath = Picker.GetPathName();
		strOriFolderPath = Picker.GetPathName();
		//디렉터리 일때
	}
	sDriveCAM = strDailyFolderPath;

	filePath = sDriveCAM + _T("\\");

	//cout << string(CT2CA(filePath)) << endl;
	m_ctrl_folder.SetWindowText(_T(" " + sDriveCAM));

	m_btn_init.EnableWindow(true);
	m_btn_start.EnableWindow(false);
	m_btn_stop.EnableWindow(false);
	m_btn_encode.EnableWindow(false);
	m_btn_framesearch.EnableWindow(false);
	m_btn_pathsetting.EnableWindow(true);
	m_btn_savebuf.EnableWindow(false);
}

void CVideorecordDlg::OnBnClickedButtonFs()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_btn_framesearch.EnableWindow(false);
	m_btn_savebuf.EnableWindow(false);

	CString M_Exp;
	CString path;
	Mat temp;
	Mat temp3;

	temp.create(SizeY1, SizeX1, CV_8UC1);
	temp3.create(SizeY1, SizeX1, CV_8UC1);

	GetDlgItemText(IDC_EDIT_FRAME, M_Exp);

	int frame_num = _ttoi(M_Exp);

	int progress = Frame_buffer.size() / 100;

	if (frame_num > imgcnt) {
		AfxMessageBox(_T("Error::(Enter a value lower than the number of grab images!)"));
		m_ctrl_log.ReplaceSel(_T("Error::(Enter a value lower than the number of grab images!)\r\n"));
		m_ctrl_log.ReplaceSel(_T("--------------------------------------------------------------------------------------------------\r\n"));
		m_btn_framesearch.EnableWindow(true);
	}
	else {
		VideoCapture cap(string(CT2CA(filePath + strTime + _T("\\") + strTime + file_format)));

		if (!cap.isOpened())
		{
			AfxMessageBox(_T("Error::(Can't Load Video)"));
			m_ctrl_log.ReplaceSel(_T("Error::(Can't Load Video)\r\n"));
			m_ctrl_log.ReplaceSel(_T("--------------------------------------------------------------------------------------------------\r\n"));
		}
		m_ctrl_log.ReplaceSel(_T("Decoding Start! \r\n"));
		m_ctrl_log.ReplaceSel(_T("--------------------------------------------------------------------------------------------------\r\n"));
		Mat Lframe;
		Lframe.create(SizeY1, SizeX1, CV_8UC1);

		while (1)
		{
			cap >> Lframe;

			if (Lframe.empty())
			{
				if (LFrame_buffer.size() == imgcnt) {
					m_ctrl_log.ReplaceSel(_T("Decoding end! \r\n"));
					m_ctrl_log.ReplaceSel(_T("--------------------------------------------------------------------------------------------------\r\n"));
					break;
				}
				else {
					m_ctrl_log.ReplaceSel(_T("Error::(Can't Load Image)\r\n"));
					m_ctrl_log.ReplaceSel(_T("--------------------------------------------------------------------------------------------------\r\n"));
					break;
				}
			}

			temp = Lframe.clone();

			LFrame_buffer.push_back(temp);

			if ((LFrame_buffer.size() % progress) == 0)
			{
				m_ctrl_progress.OffsetPos(1);
			}

			ProcessWindowMessage();
			//cout << LFrame_buffer.size() << endl;
		}

		//cout << LFrame_buffer.size() << endl;

		temp3 = LFrame_buffer[frame_num];

		MbufPutColor2d(ResultDisplaybuf, M_PACKED + M_BGR24, M_ALL_BANDS, 0, 0, SizeX1, SizeY1, (void*)temp3.data);
		// MbufPut2d(ResultDisplaybuf, 0, 0, SizeX1, SizeY1, (void*)temp3.data);

		CString file_name;

		file_name.Format(_T("Frame_%d.bmp"), frame_num);

		// MbufSave(sDriveCAM + MIL_TEXT("\\") + file_name, ResultDisplaybuf);

		imwrite(string(CT2CA(filePath + strTime + _T("\\") + file_name)), temp3);

		CString ISizeX;
		CString ISizeY;

		ISizeX.Format(_T("%d"), temp3.size().width);
		ISizeY.Format(_T("%d"), temp3.size().height);

		m_ctrl_log.ReplaceSel(_T("Selected Image Size : [ X : ") + ISizeX + _T(" , Y : ") + ISizeY + _T(" ]\r\n"));
		m_ctrl_log.ReplaceSel(_T("Selected Image Save & Load OK!\r\n"));
		m_ctrl_log.ReplaceSel(_T("--------------------------------------------------------------------------------------------------\r\n"));

		m_ctrl_progress.OffsetPos(-100);

		m_btn_framesearch.EnableWindow(true);
		m_btn_init.EnableWindow(true);
		m_btn_savebuf.EnableWindow(true);
		m_btn_pathsetting.EnableWindow(true);
		LFrame_buffer.clear();
	}

}


BOOL CVideorecordDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	if (WM_KEYDOWN == pMsg->message)
	{
		if (VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam)
		{
			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CVideorecordDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	lpMMI->ptMinTrackSize = CPoint(660, 773);
	lpMMI->ptMaxTrackSize = CPoint(660, 773);

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CVideorecordDlg::OnBnClickedButtonSave()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_btn_savebuf.EnableWindow(false);

	//CTime cTime = CTime::GetCurrentTime();
	//CString strTime = cTime.Format(_T("%Y%m%d%H%M%S"));
	CString strfilePath = filePath + strTime + _T("\\Frame");
	CString filename;
	CString fFormat = _T(".bmp");
	CString ImageNum; 
	
	int progress = Frame_buffer.size() / 100;

	if (CreateDirectory(strfilePath, NULL))
	{
		for (int i = 0; i < Frame_buffer.size(); i++)
		{
			filename.Format(_T("%d"), i + 1);
			imwrite(string(CT2CA(strfilePath + _T("\\") + filename + fFormat)), Frame_buffer[i]);

			if ((i % progress) == 0)
			{
				m_ctrl_progress.OffsetPos(1);
			}
			ProcessWindowMessage();
		}

		ImageNum.Format(_T("%d"), Frame_buffer.size());

		AfxMessageBox(_T("All Image Save OK!"));
		m_ctrl_log.ReplaceSel(_T("Total Image Save OK! - ") + ImageNum + _T("\r\n"));
		m_ctrl_log.ReplaceSel(_T("--------------------------------------------------------------------------------------------------\r\n"));
	}
	else
	{
		AfxMessageBox(_T("Error::(Can't Save Image - Directory Error)"));
		m_ctrl_log.ReplaceSel(_T("Error::(Can't Save Image - Directory Error)\r\n"));
		m_ctrl_log.ReplaceSel(_T("--------------------------------------------------------------------------------------------------\r\n"));
	}

	m_ctrl_progress.OffsetPos(-100);
	m_btn_savebuf.EnableWindow(true);
	m_btn_pathsetting.EnableWindow(true);
}
