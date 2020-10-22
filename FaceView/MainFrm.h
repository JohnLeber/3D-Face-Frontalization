
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

class CFaceViewView;
class CEffects;
class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
	CSplitterWnd m_wndSplitter;
	BOOL m_bSplitterCreated;
	IDirect3D9* md3dObject;
	D3DPRESENT_PARAMETERS md3dPP;
	CEffects* m_pEffects;
	LPDIRECT3DTEXTURE9	m_pTexture;
	void Render();
	CFaceViewView* mView[4]; // [0] Quadrant I
								// [1] Quadrant II
								// [2] Quadrant III
								// [3] Quadrant IV
// Operations
public:
	bool initD3D();

// Overrides
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CToolBar          m_wndToolBar;
	CStatusBar        m_wndStatusBar;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


