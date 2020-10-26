
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "pch.h"
#include "framework.h"
#include "FaceView.h"
#include "FaceViewDoc.h"
#include "FaceViewView.h"
#include "MainFrm.h"
#include "Effects.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IDirect3DDevice9* gd3dDevice = 0;
// CMainFrame
IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)
//-------------------------------------------------------------//
BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_DROPFILES()
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()
//-------------------------------------------------------------//
static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};
//-------------------------------------------------------------//
// CMainFrame construction/destruction
CMainFrame::CMainFrame() : m_bSplitterCreated(false)
{ 
	m_d3dObject = 0;
	m_pView[0] = m_pView[1] = m_pView[2] = m_pView[3] = 0;
}
//-------------------------------------------------------------//
CMainFrame::~CMainFrame()
{ 
	if (m_pEffects)
	{
		delete m_pEffects;
		m_pEffects = 0;
	}
	if (m_d3dObject)
	{
		m_d3dObject->Release();
	}
	if (gd3dDevice)
	{
		gd3dDevice->Release();
	}
}
//-------------------------------------------------------------//
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
	//	!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	//{
	//	TRACE0("Failed to create toolbar\n");
	//	return -1;      // fail to create
	//}

	//if (!m_wndStatusBar.Create(this))
	//{
	//	TRACE0("Failed to create status bar\n");
	//	return -1;      // fail to create
	//}
	//m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
	 
	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	//m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	//EnableDocking(CBRS_ALIGN_ANY);
	//DockControlBar(&m_wndToolBar);


	return 0;
}
//-------------------------------------------------------------//
BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	// Observe that the CSplitterWnd is a child of the frame.  The CSplitterWnd
	// is the window that basically covers the left over client area of the frame
	// window after control bars have been placed.  (The frame window includes the 
	// control bar areas.)
	m_bSplitterCreated = m_wndSplitter.CreateStatic(this, 2, 2);
	if (!m_bSplitterCreated)
		return FALSE;

	// Associate a view instance with each pane.  
	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CFaceViewView), CSize(0, 0), pContext) ||
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CFaceViewView), CSize(0, 0), pContext) ||
		!m_wndSplitter.CreateView(1, 0, RUNTIME_CLASS(CFaceViewView), CSize(0, 0), pContext) ||
		!m_wndSplitter.CreateView(1, 1, RUNTIME_CLASS(CFaceViewView), CSize(0, 0), pContext))
		return FALSE;

	// Save pointers to the views (the returned pane is usually a CView-derived class).
	m_pView[1] = (CFaceViewView*)m_wndSplitter.GetPane(0, 0);
	m_pView[0] = (CFaceViewView*)m_wndSplitter.GetPane(0, 1);
	m_pView[2] = (CFaceViewView*)m_wndSplitter.GetPane(1, 0);
	m_pView[3] = (CFaceViewView*)m_wndSplitter.GetPane(1, 1);

	return TRUE;
}
//-------------------------------------------------------------//
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	cs.cx = 1200; // width
	cs.cy = 1200; // height
	cs.y = 0; // top position
	cs.x = 0; // left position

	return TRUE;
}
//-------------------------------------------------------------//
// CMainFrame diagnostics
#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------//
// CMainFrame message handlers
bool CMainFrame::initD3D()
{
	// Step 1: Create the IDirect3D9 object.

	m_d3dObject = Direct3DCreate9(D3D_SDK_VERSION);
	if (!m_d3dObject)
	{
		TRACE0("Direct3DCreate9 FAILED");
		return false;
	}


	// Step 2: Verify hardware support for specified formats in windowed mode.

	D3DDISPLAYMODE mode;
	m_d3dObject->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
	HRESULT hr = m_d3dObject->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mode.Format, mode.Format, true);

	// Step 3: Check for hardware vertex processing.

	D3DCAPS9 caps;
	hr = m_d3dObject->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

	DWORD devBehaviorFlags = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		devBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		devBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// Step 4: Fill out the D3DPRESENT_PARAMETERS structure.  We create
	// a small backbuffer with no depth buffer.  This is because we do not
	// use the device's implicit swap chain; instead, we create a separate
	// swap chain for each of the four view windows we create.  

	m_d3dPP.BackBufferWidth = 1;
	m_d3dPP.BackBufferHeight = 1;
	m_d3dPP.BackBufferFormat = D3DFMT_UNKNOWN;
	m_d3dPP.BackBufferCount = 1;
	m_d3dPP.MultiSampleType = D3DMULTISAMPLE_NONE;
	m_d3dPP.MultiSampleQuality = 0;
	m_d3dPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_d3dPP.hDeviceWindow = 0;
	m_d3dPP.Windowed = true;
	m_d3dPP.EnableAutoDepthStencil = true;
	m_d3dPP.AutoDepthStencilFormat = D3DFMT_D16;// D3DFMT_UNKNOWN;
	m_d3dPP.Flags = 0;
	m_d3dPP.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	m_d3dPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;


	// Step 5: Create the device.

	hr = m_d3dObject->CreateDevice(
		D3DADAPTER_DEFAULT,    // primary adapter
		D3DDEVTYPE_HAL,        // device type
		GetSafeHwnd(),         // window associated with device
		devBehaviorFlags,      // vertex processing
		&m_d3dPP,               // present parameters
		&gd3dDevice);         // return created device


	// Step 6: Create a swap chain for each view.


	D3DXVECTOR3 pos, target, up;

	CRect quadrantRect;
	GetClientRect(&quadrantRect);
	float w = (float)quadrantRect.Width() / 2;
	float h = (float)quadrantRect.Height() / 2;

	InitAllVertexDeclarations(gd3dDevice);
	m_pEffects = new CEffects();
	m_pEffects->Initialize();

	m_pView[0]->Init(m_pEffects, D3DFILL_SOLID); 
	m_pView[1]->Init(m_pEffects, D3DFILL_SOLID);
	m_pView[1]->SetTextureMode();
	m_pView[2]->Init(m_pEffects, D3DFILL_SOLID);
	m_pView[2]->SetRotationMode();
	m_pView[3]->Init(m_pEffects, D3DFILL_SOLID);
	m_pView[3]->SetFaceCameraMode();
	// Step 7: For an SDI application, there is only one document object, so
	// get a pointer to it and initialize D3D objects.  (I.e., create meshes,
	// and textures, etc.)
	((CFaceViewDoc*)GetActiveDocument())->InitD3DObjects();
	DragAcceptFiles(TRUE);
	return true;
}
//-------------------------------------------------------------//
void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
	WCHAR szDroppedFile[MAX_PATH];
	long nNumFiles = DragQueryFile(hDropInfo,
		0xFFFFFFFF,
		szDroppedFile,
		MAX_PATH);
	if (nNumFiles == 1)
	{
		if (DragQueryFile(hDropInfo, 0, szDroppedFile, MAX_PATH) != 0)
		{
			CWinApp* pWinApp = AfxGetApp();
			pWinApp->OpenDocumentFile(szDroppedFile);
		}
	}
}
//-------------------------------------------------------------//
void CMainFrame::Render()
{
	if (m_pView[0]) m_pView[0]->Render();
	if (m_pView[1]) m_pView[1]->Render();
	if (m_pView[2]) m_pView[2]->Render();
	if (m_pView[3]) m_pView[3]->Render();
}
//-------------------------------------------------------------//
void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	// When the main frame window is resized, readjust the pane sizes.
	if (m_bSplitterCreated && nType != SIZE_MINIMIZED)
	{
		// The child of the frame is the CSplitterWnd.  The client area
		// of the CSplitterWnd covers the left over client area of the frame
		// window after control bars have been placed.
		CWnd* v = GetWindow(GW_CHILD);
		CRect R;
		v->GetClientRect(&R);

		int w = R.right / 2;
		int h = R.bottom / 2;

		m_wndSplitter.SetRowInfo(0, h, 1);
		m_wndSplitter.SetRowInfo(1, h, 1);
		m_wndSplitter.SetColumnInfo(0, w, 1);
		m_wndSplitter.SetColumnInfo(1, w, 1);

		m_wndSplitter.RecalcLayout();
	}
}
