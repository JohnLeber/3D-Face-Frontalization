
// FaceView.h : main header file for the FaceView application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CFaceViewApp:
// See FaceView.cpp for the implementation of this class
//

class CFaceViewApp : public CWinApp
{
public:
	CFaceViewApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	void OnFileOpen();
// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
};

extern CFaceViewApp theApp;
