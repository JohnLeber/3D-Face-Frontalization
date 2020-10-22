
// FaceViewDoc.h : interface of the CFaceViewDoc class
//
 
#pragma once
#include "Vertex.h"

class CFaceViewDoc : public CDocument
{
protected: // create from serialization only
	CFaceViewDoc();
	DECLARE_DYNCREATE(CFaceViewDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CFaceViewDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	void InitD3DObjects();
	ID3DXMesh* m_pFaceMesh;
	ID3DXMesh* m_pEyeMesh;
	LPDIRECT3DTEXTURE9 m_pTexture;
	D3DXVECTOR3 m_bbmin, m_bbmax; // bounding box.
	float m_nLeftEyeX;
	float m_nLeftEyeY;
	float m_nRightEyeX;
	float m_nRightEyeY;

	float m_nNoseX;
	float m_nNoseY; 
protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
 
	virtual void OnCloseDocument();
};
