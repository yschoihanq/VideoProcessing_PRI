
// VideorecordDlg.h: 헤더 파일
//

#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/core/core.hpp>+
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <Mil.h>
#include <string>

// CVideorecordDlg 대화 상자
class CVideorecordDlg : public CDialogEx
{
// 생성입니다.
public:
	CVideorecordDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIDEORECORD_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_ctrl_disp;
	afx_msg void OnBnClickedButtonInit();
	CEdit m_ctrl_log;
	CButton m_btn_init;
	CButton m_btn_start;
	CButton m_btn_stop;
	CButton m_btn_framesearch;
	CButton m_btn_pathsetting;
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	CButton m_btn_encode;
	afx_msg void OnBnClickedButtonEncode();
	afx_msg void OnBnClickedButtonPs();
	CStatic m_ctrl_folder;

	afx_msg void OnBnClickedButtonFs();
	CEdit m_edit_frame;
	CStatic m_ctrl_disp2;

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	CButton m_btn_savebuf;
	afx_msg void OnBnClickedButtonSave();
	CProgressCtrl m_ctrl_progress;
};
