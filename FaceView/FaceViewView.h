
// FaceViewView.h : interface of the CFaceViewView class
//

#pragma once
#include <d3d9.h>
#include <d3dx9.h> 
#include "vertex.h" 
 
//--------------------------------------------------------------------------------------------------//
struct PPVERT
{
	float x, y, z, rhw;
	float tu, tv;       // Texcoord for post-process source
	float tu2, tv2;     // Texcoord for the original scene
	static IDirect3DVertexDeclaration9* Decl;
};
class CEffects;
class DrawableTex2D;
class CFaceViewView : public CView
{
protected: // create from serialization only
	CFaceViewView();
	DECLARE_DYNCREATE(CFaceViewView)
	
// Attributes
public:
	CFaceViewDoc* GetDocument() const;

// Operations
public:
	void SetTextureMode();//display pDoc->m_pTexture
	void SetRotationMode();
	void SetFaceCameraMode();
	LONG GetDisplayWidth();
	LONG GetDisplayHeight();
	
// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CFaceViewView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	IDirect3DSwapChain9* m_SwapChain;
	IDirect3DSurface9* m_DepthStencil;
	D3DPRESENT_PARAMETERS m_d3dPP;
	D3DXMATRIX m_World, m_WVP, m_WI, m_WIT, m_View, m_Proj;
	D3DFILLMODE m_Fillmode;
	bool m_bSetTextureMode, m_bRotate, m_bFaceCameraMode;
	CEffects* m_pEffects;
	DrawableTex2D* m_pDepthMap;
	HRESULT DrawTexturedQuad(LPDIRECT3DDEVICE9 pDevice,
		FLOAT x, FLOAT y, FLOAT z,
		FLOAT width, FLOAT height,
		D3DXVECTOR2 uvTopLeft, D3DXVECTOR2 uvTopRight,
		D3DXVECTOR2 uvBottomLeft, D3DXVECTOR2 uvBottomRight,
		LPDIRECT3DTEXTURE9 pTexture);

	HRESULT DrawTransformedQuad(LPDIRECT3DDEVICE9 pDevice,
		FLOAT x, FLOAT y, FLOAT z,
		FLOAT width, FLOAT height,
		D3DXVECTOR2 uvTopLeft, D3DXVECTOR2 uvTopRight,
		D3DXVECTOR2 uvBottomLeft, D3DXVECTOR2 uvBottomRight,
		D3DCOLOR c1, D3DCOLOR c2, D3DCOLOR c3, D3DCOLOR c4);

	HRESULT DrawTransformedQuad(LPDIRECT3DDEVICE9 pDevice, FLOAT x, FLOAT y, FLOAT z,
		FLOAT width, FLOAT height,
		D3DCOLOR c1 = 0xFFFFFFFF,
		D3DCOLOR c2 = 0xFFFFFFFF,
		D3DCOLOR c3 = 0xFFFFFFFF,
		D3DCOLOR c4 = 0xFFFFFFFF);

	bool CalcMarkerPos(float nEyeX, float nEyeY, D3DXMATRIX lightView, D3DXMATRIX lightLens, FLOAT& X, FLOAT& Y, FLOAT& Z);

	void DrawMarker(float nEyeX, float nEyeY, D3DXMATRIX R, D3DXMATRIX lightView, D3DXMATRIX lightLens, ID3DXMesh* pMesh);
// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	void Init(CEffects* pEffects, D3DFILLMODE defaultFillmode);
	void ReCreateBuffers(int w, int h);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void Render();
};

#ifndef _DEBUG  // debug version in FaceViewView.cpp
inline CFaceViewDoc* CFaceViewView::GetDocument() const
   { return reinterpret_cast<CFaceViewDoc*>(m_pDocument); }
#endif

