
// FaceViewDoc.cpp : implementation of the CFaceViewDoc class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "FaceView.h"
#endif

#include "FaceViewDoc.h"
# define LINE_BUFF_SIZE 4096
#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFaceViewDoc

IMPLEMENT_DYNCREATE(CFaceViewDoc, CDocument)

BEGIN_MESSAGE_MAP(CFaceViewDoc, CDocument)
END_MESSAGE_MAP()


// CFaceViewDoc construction/destruction

CFaceViewDoc::CFaceViewDoc()
{
	m_pFaceMesh = 0;
	m_pEyeMesh = 0;
	m_pTexture = 0;
	m_nLeftEyeX = 0;
	m_nLeftEyeY = 0;
	m_nRightEyeX = 0;
	m_nRightEyeY = 0;
}

CFaceViewDoc::~CFaceViewDoc()
{
	if (m_pTexture)
	{
		m_pTexture->Release();
		m_pTexture = 0;
	}
	if (m_pFaceMesh)
	{
		m_pFaceMesh->Release();
		m_pFaceMesh = 0;
	}
	if (m_pEyeMesh)
	{
		m_pEyeMesh->Release();
		m_pEyeMesh = 0;
	}
}

BOOL CFaceViewDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	 
	return TRUE;
}
 
// CFaceViewDoc serialization

void CFaceViewDoc::Serialize(CArchive& ar)
{
	//if (ar.IsStoring())
	//{
	//	// TODO: add storing code here
	//}
	//else
	//{
	//	// TODO: add loading code here
	//}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CFaceViewDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CFaceViewDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data.
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CFaceViewDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CFaceViewDoc diagnostics

#ifdef _DEBUG
void CFaceViewDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFaceViewDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------//
// CFaceViewDoc commands
void CFaceViewDoc::InitD3DObjects()
{
	//HRESULT hr = D3DXCreateTeapot(gd3dDevice, &mTeapot, 0);
}
//-------------------------------------------------------------//
BOOL CFaceViewDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	CString strObjFile = lpszPathName;
	if (strObjFile.Right(4) != L".obj" && strObjFile.Right(5) != L".mesh")
	{
		AfxMessageBox(L"Not and mesh file");
		return FALSE;
	}
	if (m_pEyeMesh)
	{
		m_pEyeMesh->Release();
		m_pEyeMesh = 0;
	}
	HRESULT hr = D3DXCreateBox(gd3dDevice, 0.005f, 0.005f, 0.005f, &m_pEyeMesh, NULL);

	m_nLeftEyeX = 0;
	m_nLeftEyeY = 0;
	m_nRightEyeX = 0;
	m_nRightEyeY = 0;
	m_nNoseX = 0;
	m_nNoseY = 0;

	//load textures 
	if (m_pTexture)
	{
		m_pTexture->Release();
		m_pTexture = 0;
	}
	{
		CString strTextureFile = strObjFile;
		strTextureFile.Replace(L"_mesh.mesh", L".jpg");

		hr = D3DXCreateTextureFromFile(gd3dDevice, strTextureFile, &m_pTexture);
		if (FAILED(hr))
		{

		}
	}
	//read from the files_eyetest._landmarks
	CHAR buffer[LINE_BUFF_SIZE];
	{
		CString strLandmarkFile = strObjFile;
		strLandmarkFile.Replace(L"_mesh.mesh", L"_landmarks.txt");
		FILE* pFile = 0;
		errno_t ret = _tfopen_s(&pFile, strLandmarkFile, TEXT("r"));
		if (ret == 0)
		{
			int nLandmarkID = 0;
			while (!feof(pFile))
			{
				fgets(buffer, LINE_BUFF_SIZE, pFile);
				if (nLandmarkID == 0)
				{
					sscanf_s(buffer + 1, "%f %f", &m_nLeftEyeX, &m_nLeftEyeY);
				}
				else if (nLandmarkID == 1)
				{
					sscanf_s(buffer + 1, "%f %f", &m_nRightEyeX, &m_nRightEyeY);
				}
				else if (nLandmarkID == 2)
				{
					sscanf_s(buffer + 1, "%f %f", &m_nNoseX, &m_nNoseY);
				}
				nLandmarkID++;
			}
			fclose(pFile);
		}
		else
		{
			ATLASSERT(0);//no landmark file
		}
	}

	if (m_pFaceMesh)
	{
		m_pFaceMesh->Release();
		m_pFaceMesh = 0;
	}
	INT nF1 = 0;
	INT nF2 = 0;
	INT nF3 = 0;
	//m_pFaceMesh
	 
	FILE* pFile = 0;
	errno_t ret = _tfopen_s(&pFile, lpszPathName, TEXT("r"));
	if (ret != 0) return FALSE;
	int numVertices = 0;
	int numFaces = 0;
	while (!feof(pFile))
	{
		buffer[0] = 0;
		fgets(buffer, LINE_BUFF_SIZE, pFile);
		if (0 == strncmp("v ", buffer, 2))			numVertices++;
		else if (0 == strncmp("f ", buffer, 2))
		{
			numFaces++;
		}
	}
	if(numVertices == 0 || numFaces == 0)
	{ 
		fclose(pFile); 
		return FALSE;
	} 
	rewind(pFile);
	std::vector<D3DXVECTOR3> vVertices;
	std::vector<INT> vIndices;
	vVertices.reserve(numVertices);
	vIndices.reserve(numFaces * 3);
	long nVertexCounter = 0;
	while (!feof(pFile))
	{
		buffer[0] = '\0';
		fgets(buffer, LINE_BUFF_SIZE, pFile); 
		if (0 == strncmp("v ", buffer, 2))
		{
			nVertexCounter++;
			D3DXVECTOR3 v; 
			sscanf_s(buffer + 1, "%f %f %f", &v.x, &v.y, &v.z);
			v.x = -v.x / 8.0f;
			v.y = v.y / 8.0f;
			v.z = v.z / 8.0f;
			vVertices.push_back(v);
			if (nVertexCounter == 1) {
				m_bbmin = m_bbmax = v;
			}
			else
			{
				if (v.x < m_bbmin.x) m_bbmin.x = v.x; else if (v.x > m_bbmax.x) m_bbmax.x = v.x;
				if (v.y < m_bbmin.y) m_bbmin.y = v.y; else if (v.y > m_bbmax.y) m_bbmax.y = v.y;
				if (v.z < m_bbmin.z) m_bbmin.z = v.z; else if (v.z > m_bbmax.z) m_bbmax.z = v.z;
			}
		}
		else if (0 == strncmp("f ", buffer, 2))
		{
			sscanf_s(buffer + 1, "%d %d %d", &nF1, &nF2, &nF3);
			vIndices.push_back(nF1 - 1);
			vIndices.push_back(nF2 - 1);
			vIndices.push_back(nF3 - 1);
		}
	}
	fclose(pFile);

	D3DVERTEXELEMENT9 VertexPNTElements[] =
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};

	bool bFlipTriangles = true;
	hr = D3DXCreateMesh(vIndices.size() / 3, vVertices.size(), D3DXMESH_MANAGED | D3DXMESH_32BIT, VertexPNTElements, gd3dDevice, &m_pFaceMesh);
	if (SUCCEEDED(hr))
	{
		IDirect3DVertexBuffer9* pAreaVB = 0;
		IDirect3DIndexBuffer9* pAreaIB = 0;

		m_pFaceMesh->GetVertexBuffer(&pAreaVB);
		VertexPNT* v = 0;
		long nNumVertices = vVertices.size();
		pAreaVB->Lock(0, 0, (void**)&v, 0);
		for (int h = 0; h < nNumVertices; h++)
		{
			D3DXVECTOR3 pos = vVertices[h];
			v[h] = VertexPNT(pos.x, pos.y, pos.z, 0, 0, 0, 0, 0);
		}

		pAreaVB->Unlock();
		pAreaVB->Release();

		m_pFaceMesh->GetIndexBuffer(&pAreaIB);
		DWORD* i = 0;
		long nCount = 0;
		pAreaIB->Lock(0, 0, (void**)&i, 0);
		for (UINT h = 0; h < vIndices.size() / 3; h++)
		{
			if (bFlipTriangles)
			{
				i[3 * h + 0] = vIndices[3 * h];
				i[3 * h + 2] = vIndices[3 * h + 1];
				i[3 * h + 1] = vIndices[3 * h + 2];
			}
			else
			{
				i[3 * h + 0] = vIndices[3 * h];
				i[3 * h + 1] = vIndices[3 * h + 1];
				i[3 * h + 2] = vIndices[3 * h + 2];
			}
		}
		pAreaIB->Unlock();
		pAreaIB->Release();
		long ert = 0;
	} 
	return TRUE;
}
//-------------------------------------------------------------//
void CFaceViewDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class

	CDocument::OnCloseDocument();
}
