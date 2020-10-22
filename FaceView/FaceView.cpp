
// FaceView.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "FaceView.h"
#include "MainFrm.h"

#include "FaceViewDoc.h"
#include "FaceViewView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFaceViewApp

BEGIN_MESSAGE_MAP(CFaceViewApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CFaceViewApp::OnAppAbout)
	// Standard file based document commands 
	ON_COMMAND(ID_FILE_OPEN, &CFaceViewApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


// CFaceViewApp construction

CFaceViewApp::CFaceViewApp() noexcept
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("FaceView.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CFaceViewApp object

CFaceViewApp theApp;


// CFaceViewApp initialization

BOOL CFaceViewApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CFaceViewDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CFaceViewView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The m_pMainWnd becomes valid after ProcessShellCommand.
	if (!((CMainFrame*)m_pMainWnd)->initD3D())
	{
		TRACE0("initD3D Failed");
		return FALSE;
	}

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

BOOL CFaceViewApp::OnIdle(LONG lCount)
{
	CWinApp::OnIdle(lCount);

	CWnd* mainFrame = AfxGetMainWnd();

	 

	// Save last time.
	//static float lastTime = (float)timeGetTime();
	// Compute time now.
	//float currentTime = (float)timeGetTime();
	// Compute the difference: time elapsed in seconds.
	//float deltaTime = (currentTime - lastTime) * 0.001f;
	// Last time is now current time.
	//lastTime = currentTime;
	//cview->update(deltaTime);
	//cview->render();

	((CMainFrame*)mainFrame)->Render();
	//cview->render();

	return TRUE;
}
int CFaceViewApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinApp::ExitInstance();
}

// CFaceViewApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CFaceViewApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CFaceViewApp message handlers 
void CFaceViewApp::OnFileOpen()
{
	TCHAR szFilters[] = _T("OBJ MESH Files (*.mesh)|*.mesh|All Files (*.*)|*.*||"); 
	CFileDialog d(TRUE, _T("mesh"), _T("*.mesh"), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters, AfxGetMainWnd());
	if (d.DoModal() != IDOK) return;
	CString cstrFileOpen = d.GetPathName();
	 
	// Call CImageEngDoc :: OnOpenDocument() automatically
	OpenDocumentFile(cstrFileOpen);
}

CDocument* CFaceViewApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	// TODO: Add your specialized code here and/or call the base class
	return CWinApp::OpenDocumentFile(lpszFileName);
}
