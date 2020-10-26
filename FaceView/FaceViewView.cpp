
// FaceViewView.cpp : implementation of the CFaceViewView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "FaceView.h"
#endif

#include "FaceViewDoc.h"
#include "FaceViewView.h"
#include "DrawableTex2D.h"
#include "Effects.h"
#include "Vertex.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif 

const float gTextureDim = 224.0f;//The Microsoft Deep3DFaceReconstruction project outputs a cropped image as 224 by 224 (as part of the CNN output)

 
// CFaceViewView

IMPLEMENT_DYNCREATE(CFaceViewView, CView)

BEGIN_MESSAGE_MAP(CFaceViewView, CView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()
//--------------------------------------------------------------------------------------------------//
// CFaceViewView construction/destruction
CFaceViewView::CFaceViewView() : m_SwapChain(0), m_DepthStencil(0)
{
	m_pEffects = 0;
 
	m_d3dPP.BackBufferWidth = 0;
	m_d3dPP.BackBufferHeight = 0;
	m_d3dPP.BackBufferFormat = D3DFMT_UNKNOWN;
	m_d3dPP.BackBufferCount = 1;
	m_d3dPP.MultiSampleType = D3DMULTISAMPLE_NONE;
	m_d3dPP.MultiSampleQuality = 0;
	m_d3dPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_d3dPP.hDeviceWindow = GetSafeHwnd();
	m_d3dPP.Windowed = true;
	m_d3dPP.EnableAutoDepthStencil = true;
	m_d3dPP.AutoDepthStencilFormat = D3DFMT_D16;// D3DFMT_D24S8;
	m_d3dPP.Flags = 0;
	m_d3dPP.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	m_d3dPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	m_bSetTextureMode = false;
	m_bRotate = false;
	m_bFaceCameraMode = false;
	m_pDepthMap = 0;
}
//--------------------------------------------------------------------------------------------------//
CFaceViewView::~CFaceViewView()
{
	ReleaseCOM(m_SwapChain);
	ReleaseCOM(m_DepthStencil);
	if (m_pDepthMap)
	{
		delete m_pDepthMap;
		m_pDepthMap = 0;
	}
}
//--------------------------------------------------------------------------------------------------//
BOOL CFaceViewView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}
//--------------------------------------------------------------------------------------------------//
void CFaceViewView::SetTextureMode()
{
	m_bSetTextureMode = true;
	m_bFaceCameraMode = false;
	m_bRotate = false;
}
//--------------------------------------------------------------------------------------------------//
void CFaceViewView::SetRotationMode()
{
	m_bRotate = true;
	m_bFaceCameraMode = false;
	m_bSetTextureMode = false;
}
//--------------------------------------------------------------------------------------------------//
void CFaceViewView::SetFaceCameraMode()
{
	m_bFaceCameraMode = true;
	m_bSetTextureMode = false;
	m_bRotate = false;
}
//--------------------------------------------------------------------------------------------------//
// CFaceViewView drawing
LONG CFaceViewView::GetDisplayWidth()
{
	CRect rtClient;
	GetClientRect(rtClient);
	return rtClient.Width();
}
//--------------------------------------------------------------------------------------------------//
LONG CFaceViewView::GetDisplayHeight()
{
	CRect rtClient;
	GetClientRect(rtClient);
	return rtClient.Height();
}
//--------------------------------------------------------------------------------------------------//
bool CFaceViewView::CalcMarkerPos(float nEyeX, float nEyeY, D3DXMATRIX lightView, D3DXMATRIX lightLens, FLOAT& X, FLOAT& Y, FLOAT& Z)
{
	CRect R;
	GetClientRect(&R);
	bool bHitResult = false;
	CFaceViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) {
		return false;
	}

	{
		float w = (float)R.Width();
		float h = (float)R.Height();

		D3DXVECTOR3 originW(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 dirW(0.0f, 0.0f, 0.0f);

		POINT s;
		s.x = static_cast<LONG>(nEyeX * w);
		s.y = static_cast<LONG>(nEyeY * h);
		D3DXMATRIX proj = lightLens;

		float x = (2.0f * s.x / w - 1.0f) / proj(0, 0);
		float y = (-2.0f * s.y / h + 1.0f) / proj(1, 1);

		// Build picking ray in view space.
		D3DXVECTOR3 origin(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 dir(x, y, 1.0f);

		//// So if the view matrix transforms coordinates from 
		//// world space to view space, then the inverse of the
		//// view matrix transforms coordinates from view space
		//// to world space.
		D3DXMATRIX invView;
		D3DXMatrixInverse(&invView, 0, &lightView);

		//// Transform picking ray to world space.
		D3DXVec3TransformCoord(&originW, &origin, &invView);
		D3DXVec3TransformNormal(&dirW, &dir, &invView);
		D3DXVec3Normalize(&dirW, &dirW);
		ID3DXBuffer* pAllHits = 0;
		DWORD dwFaceIndex = -1;

		D3DXVECTOR4 vorgW;
		D3DXMATRIX Tinv;
		D3DXMATRIX T;
		D3DXMatrixIdentity(&T);
		D3DXMatrixInverse(&Tinv, 0, &T);
		D3DXVec3Transform(&vorgW, &originW, &Tinv);
		D3DXVECTOR3 originW2(vorgW.x, vorgW.y, vorgW.z);
		BOOL bHit = FALSE;
		DWORD dwCount = 0;
		FLOAT nU = 0;
		FLOAT nV = 0;
		FLOAT nDist = 0;
		//calulcate where the ray intersects with the face mesh
		HRESULT hr = D3DXIntersect(pDoc->m_pFaceMesh, &originW2, &dirW, &bHit, &dwFaceIndex, &nU, &nV, &nDist, &pAllHits, &dwCount);
		if (SUCCEEDED(hr) && bHit)
		{
			DWORD* pIndices;
			IDirect3DVertexBuffer9* vb = 0;
			IDirect3DIndexBuffer9* ib = 0;
			hr = pDoc->m_pFaceMesh->GetVertexBuffer(&vb);
			hr = pDoc->m_pFaceMesh->GetIndexBuffer(&ib);
			VertexPNT* pVertices;
			ib->Lock(0, 0, (void**)&pIndices, 0);
			vb->Lock(0, 0, (void**)&pVertices, 0);

			hr = gd3dDevice->SetIndices(ib);
			hr = gd3dDevice->SetVertexDeclaration(VertexPNT::Decl);
			hr = gd3dDevice->SetStreamSource(0, vb, 0, sizeof(VertexPNT));
			VertexPNT vThisTri[3];

			DWORD* nThisTri;
			nThisTri = &pIndices[dwFaceIndex * 3];

			vThisTri[0] = pVertices[nThisTri[0]];
			vThisTri[1] = pVertices[nThisTri[1]];
			vThisTri[2] = pVertices[nThisTri[2]];

			vThisTri[0].pos.x;
			vThisTri[0].pos.y;
			float w = 1 - (nU + nV);
			//finally get the xyz world coordintates and return them to the caller
			X = (w * vThisTri[0].pos.x + nU * vThisTri[1].pos.x + nV * vThisTri[2].pos.x);
			Y = (w * vThisTri[0].pos.y + nU * vThisTri[1].pos.y + nV * vThisTri[2].pos.y);
			Z = (w * vThisTri[0].pos.z + nU * vThisTri[1].pos.z + nV * vThisTri[2].pos.z);
			bHitResult = true;
			ib->Unlock();
			vb->Unlock();
			ib->Release();
			vb->Release();
		}
		if (pAllHits)
		{
			pAllHits->Release();
			pAllHits = 0;
		}
	}
	return bHitResult;
}
//--------------------------------------------------------------------------------------------------//
void CFaceViewView::DrawMarker(float nEyeX, float nEyeY, D3DXMATRIX Rotation, D3DXMATRIX lightView, D3DXMATRIX lightLens, ID3DXMesh* pMesh)
{
	CFaceViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) {
		return;
	}
	FLOAT X = 0;
	FLOAT Y = 0;
	FLOAT Z = 0;
	if (!CalcMarkerPos(nEyeX, nEyeY, lightView, lightLens, X, Y, Z))
	{
		return;
	}
	D3DXMATRIX T;
	D3DXMatrixTranslation(&T, X, Y, Z);
	m_World = T;
	m_World *= Rotation;
	m_WVP = m_World * m_View * m_Proj;
	D3DXMatrixInverse(&m_WI, NULL, &m_World);
	D3DXMatrixTranspose(&m_WIT, &m_WI);
 
	m_pEffects->m_pFX->SetMatrix("mWorld", &m_World);
	m_pEffects->m_pFX->SetMatrix("mWVP", &m_WVP);
	m_pEffects->m_pFX->SetMatrix("mWIT", &m_WIT);

	HRESULT hr = m_pEffects->m_pFX->SetTexture(m_pEffects->m_hTexture, pDoc->m_pTexture);

	m_pEffects->m_pFX->CommitChanges();
	pMesh->DrawSubset(0);
}
//--------------------------------------------------------------------------------------------------//
void CFaceViewView::Render()
{
	CFaceViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) {
		return;
	}
	//Note: in the code below, 'light' (as in lightlens, lightView etc) refers to the position of the camera projecting the texture onto the mesh.
	//This corresponds to the 'camera' referred to in the Microsoft Deep3DFaceReconstruction github project that has a FOV of 12.59 at is located 10 m in front of the face mesh.
	//It is analgous to a light source when calculating the shadow map (for example), but it isn't really a light source.
	D3DXVECTOR3 lightPosW(0, 0, 10);
	D3DXMATRIX lightWVP;
	float nNearPlane = 1.0f;
	float nFarPlane = 50.0f;
	float nfFOV = static_cast<float>(2 * atan(112.0f / (1015.0f)) * 180.0f / D3DX_PI);
	if (gd3dDevice)
	{
		gd3dDevice->SetRenderState(D3DRS_FILLMODE, m_Fillmode);
		// Render to the view's local swap chain and depth/stencil buffer.
		IDirect3DSurface9* backbuffer = 0;
		HRESULT hr = m_SwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
		hr = gd3dDevice->SetRenderTarget(0, backbuffer);
		hr = gd3dDevice->SetDepthStencilSurface(m_DepthStencil);
		ReleaseCOM(backbuffer);
	 
		if (pDoc && pDoc->m_pFaceMesh && !m_bSetTextureMode)
		{
			//create the shadow map used for shadowing ( as displayed in red on rendered mask)
			m_pDepthMap->beginScene();
			HRESULT hr = gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
			hr = m_pEffects->m_pFX->SetTechnique(m_pEffects->m_hDepthTech);
			D3DXMATRIX lightView;
			D3DXVECTOR3 UpW(0.0f, 1.0f, 0.0f);//specify up direction
			D3DXMatrixLookAtLH(&lightView, &lightPosW, &(D3DXVECTOR3(0, 0, 0)), &UpW);
			D3DXMATRIX lightLens;
			float LightFOV = static_cast<float>(2 * atan(112.0f / (1015.0f)) * 180.0f / D3DX_PI);//12.59 - see MS Deep3D project
			D3DXMatrixPerspectiveFovLH(&lightLens, LightFOV, 1, nNearPlane, nFarPlane);//used to be 100
			lightWVP = lightView * lightLens;
			UINT numPasses = 0;
			hr = m_pEffects->m_pFX->Begin(&numPasses, 0);
			hr = m_pEffects->m_pFX->BeginPass(0);
			hr = m_pEffects->m_pFX->SetTexture(m_pEffects->m_hShadowMap, m_pDepthMap->d3dTex());
			hr = m_pEffects->m_pFX->SetFloatArray("vEye", &lightPosW.x, 3);
			hr = m_pEffects->m_pFX->SetValue(m_pEffects->m_hFarPlane, &nFarPlane, sizeof(float));
			hr = m_pEffects->m_pFX->SetMatrix(m_pEffects->m_hLightWVP, &(lightWVP));
			hr = m_pEffects->m_pFX->CommitChanges();
			//draw the mesh
			hr = pDoc->m_pFaceMesh->DrawSubset(0);
			hr = m_pEffects->m_pFX->EndPass();
			hr = m_pEffects->m_pFX->End();
			m_pDepthMap->endScene();
		}
	 	hr = gd3dDevice->BeginScene();
		hr = gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(128, 128, 128), 1.0, 0);

		// Draw the background gradient.
		RECT rClient;
		GetClientRect(&rClient);
		gd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		{
			D3DXCOLOR c1(0.25f, 0.25f, 0.25f, 1.0f);
			D3DXCOLOR c2(0.43f,0.43f, 0.43f, 1.0f);
			gd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			DrawTransformedQuad(gd3dDevice, -0.5f, -0.5f, 0.f, (FLOAT)(rClient.right - rClient.left),
				(FLOAT)(rClient.bottom - rClient.top),
				c1, c1, c2, c2);
			gd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		}
		if (m_bSetTextureMode)
		{
			if (pDoc->m_pTexture)
			{
				hr = gd3dDevice->SetVertexDeclaration(VertexSS::Decl);
				gd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
				DrawTexturedQuad(gd3dDevice, -0.5f, -0.5f, 0.f, (FLOAT)(rClient.right - rClient.left),
					(FLOAT)(rClient.bottom - rClient.top),
					D3DXVECTOR2(0, 0), D3DXVECTOR2(1, 0), D3DXVECTOR2(0, 1), D3DXVECTOR2(1, 1),
					pDoc->m_pTexture);
				gd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			}
		}
		else
		{			 
			// Draw the scene data stored in the document class. 
			gd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW); 
			if (pDoc->m_pFaceMesh)
			{
				FLOAT scaleFactor = 1;
				D3DXMATRIX Translate, Rotation, Scale;
				D3DXMatrixTranslation(&Translate, 0, 0, 0);// -bbCenter.x, -bbCenter.y, -bbCenter.z );
				D3DXMatrixScaling(&Scale, scaleFactor, scaleFactor, scaleFactor);
				float nRotationY = 0.0f;
				float nRotationZ = 0.0f;
				if (m_bRotate) {
					nRotationY = GetTickCount() / 1000.f;
				}
				else if (m_bFaceCameraMode)
				{
					RotateTowardsCamera(lightPosW, nNearPlane, nFarPlane, nRotationY, nRotationZ);					 
				}
				
				D3DXMatrixRotationYawPitchRoll(&Rotation, nRotationY, 0, nRotationZ);
				m_World = Translate * Scale * Rotation;
				//Note, when rendering, we do not need to use the same 12.59 FOV as we need to use when projecting the texture onto the mesh (see the MS Deep3D... project), but we'll re-use it here anyway
				float nRenderFOV = 12.59f;
				D3DXVECTOR3 vEye(0, 0, 10);//the eye position that we view the meshes when we render them. 
				//Note, this can be anything, that is it dones't have to be 10 units in front like it needs to be when projecting the texture onto the face. But like the FOV we'll resuse it here anyway.
				hr = gd3dDevice->SetVertexDeclaration(VertexPNT::Decl);
				gd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
				hr = m_pEffects->m_pFX->SetTexture(m_pEffects->m_hTexture, pDoc->m_pTexture);
				hr = m_pEffects->m_pFX->SetTexture(m_pEffects->m_hShadowMap, m_pDepthMap->d3dTex());
				hr = m_pEffects->m_pFX->SetTechnique(m_pEffects->m_hProjTech);
				
				D3DXMatrixLookAtLH(&m_View, &vEye, &(D3DXVECTOR3(0, 0, 0)), &(D3DXVECTOR3(0, 1, 0)));
				D3DXMatrixPerspectiveFovLH(&m_Proj, nRenderFOV, 1, nNearPlane, nFarPlane);
				m_WVP = m_World * m_View * m_Proj;
				D3DXMatrixInverse(&m_WI, NULL, &m_World);
				D3DXMatrixTranspose(&m_WIT, &m_WI);

				UINT passes = 0;
				if (SUCCEEDED(m_pEffects->m_pFX->Begin(&passes, 0)))
				{
					m_pEffects->m_pFX->BeginPass(0);

					m_pEffects->m_pFX->SetMatrix("mWorld", &m_World);
					m_pEffects->m_pFX->SetMatrix("mWVP", &m_WVP);
					m_pEffects->m_pFX->SetMatrix("mWIT", &m_WIT);

					D3DXMATRIX lightView;
					D3DXMATRIX lightLens;
					D3DXVECTOR3 UpW(0.0f, 1.0f, 0.0f);
					{
						D3DXMatrixLookAtLH(&lightView, &lightPosW, &(D3DXVECTOR3(0, 0, 0)), &UpW);
						float ProjFOV = static_cast<float>((float)2 * atan(112.0f / (1015.0f)) * 180.0f / D3DX_PI);
						D3DXMatrixPerspectiveFovLH(&lightLens, ProjFOV, 1, nNearPlane, nFarPlane);//used to be 100
						m_pEffects->m_pFX->SetMatrix(m_pEffects->m_hLightWVP, &(lightWVP));
					}
					m_pEffects->m_pFX->SetValue(m_pEffects->m_hFarPlane, &nFarPlane, sizeof(float));
					m_pEffects->m_pFX->SetFloatArray("vEye", &vEye.x, 3);
					m_pEffects->m_pFX->CommitChanges();

					hr = pDoc->m_pFaceMesh->DrawSubset(0);
					m_pEffects->m_pFX->EndPass();
					m_pEffects->m_pFX->End();

					{
						//display the left and right eye as a blue mesh
						HRESULT hr = m_pEffects->m_pFX->SetTechnique(m_pEffects->m_hColorTech);
						if (SUCCEEDED(m_pEffects->m_pFX->Begin(&passes, 0)))
						{
							m_pEffects->m_pFX->BeginPass(0);
							float nEyeX = pDoc->m_nLeftEyeX / gTextureDim;
							float nEyeY = pDoc->m_nLeftEyeY / gTextureDim;
							DrawMarker(nEyeX, nEyeY, Rotation, lightView, lightLens, pDoc->m_pEyeMesh);
							nEyeX = pDoc->m_nRightEyeX / gTextureDim;
							nEyeY = pDoc->m_nRightEyeY / gTextureDim;
							DrawMarker(nEyeX, nEyeY, Rotation, lightView, lightLens, pDoc->m_pEyeMesh);

							/*float nNoseX = pDoc->m_nNoseX / 224.0f;
							float nNoseY = pDoc->m_nNoseY / 224.0f;
							DrawMarker(nNoseX, nNoseY, Rotation, lightView, lightLens, pDoc->m_pEyeMesh);*/

							m_pEffects->m_pFX->EndPass();
							m_pEffects->m_pFX->End();
						}
					}				 
				} 
			}
		}

		hr = gd3dDevice->EndScene();

		// We override the window whose client area is taken as the target
		// for this presentation.  That is, we use the CMainFrame window's
		// HWND to initialize D3D, but we draw into the view windows.
		// Also note that we use the swap chain's present method.
		hr = m_SwapChain->Present(0, 0, GetSafeHwnd(), 0, 0);
		gd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
}
//--------------------------------------------------------------------------------------------------//
void CFaceViewView::RotateTowardsCamera(D3DXVECTOR3 lightPosW, float nNear, float nFar, float& nRotationY, float& nRotationZ)
{
	CFaceViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) {
		return;
	}
	//rotate mesh so that the face is alignbed with the X/Y plane
	D3DXMATRIX lightView;
	D3DXMATRIX lightLens;
	D3DXVECTOR3 UpW(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&lightView, &lightPosW, &(D3DXVECTOR3(0, 0, 0)), &UpW);
	float ProjFOV = static_cast<float>(2 * atan(112.0f / (1015.0f)) * 180.0f / D3DX_PI);//FOV used from creating the mesh/texture in the MS Deep3DFaceReconstruction.
	D3DXMatrixPerspectiveFovLH(&lightLens, ProjFOV, 1, nNear, nFar);
	FLOAT nLeftX = 0;
	FLOAT nLeftY = 0;
	FLOAT nLeftZ = 0;
	FLOAT nRightX = 0;
	FLOAT nRightY = 0;
	FLOAT nRightZ = 0;
	float nEyeX = pDoc->m_nLeftEyeX / gTextureDim;
	float nEyeY = pDoc->m_nLeftEyeY / gTextureDim;
	//project landmark values for left/right eye from cropped (224 x 224) 2D texture space to 3D world space
	if (CalcMarkerPos(nEyeX, nEyeY, lightView, lightLens, nLeftX, nLeftY, nLeftZ))
	{
		nEyeX = pDoc->m_nRightEyeX / gTextureDim;
		nEyeY = pDoc->m_nRightEyeY / gTextureDim;
		if (CalcMarkerPos(nEyeX, nEyeY, lightView, lightLens, nRightX, nRightY, nRightZ))
		{
			//found XYZ coord of both left and right eye
			//Use these to work out what the rotation should be such that both eyes are aligned to the view plane.
			//Y is Up, so find angles on the X/Z plane.
			float nLeftHypot = static_cast<float>(_hypot(nLeftX, nLeftZ));
			float nRightHypot = static_cast<float>(_hypot(nRightX, nRightZ));
			float nLeftAngle = static_cast<float>(atan2(nLeftZ, nLeftX));
			float nRightAngle = static_cast<float>(atan2(nRightZ, nRightX));
			float nCenterAngle = (nRightAngle + nLeftAngle) / 2;
			float nLeftAngleDegrees = nLeftAngle / D3DX_PI * 180;
			float nRightAngleDegrees = nRightAngle / D3DX_PI * 180;
			float nCenterAngleDegrees = nCenterAngle / D3DX_PI * 180;
			//float nRotationDegrees = nRotationY / 3.D3DX_PI * 180;
			//We use a numerical approach to find an angle where the Z value on both eyes are closest (rather than try and documenta an analytical approach)
			//We do a sweep from -90 degree of the current angle to +90 degrees and take the angle with the two new points arethe same
			float nSmallest = -1;
			float nSmallestRad = 0;
			for (int h = 0; h < 180; h++)
			{
				float nRad = (h - 90) * D3DX_PI / 180.0f;
				float nNewLeftZ = static_cast<float>(nLeftHypot * sin(nRad + nLeftAngle));
				float nNewRightZ = static_cast<float>(nRightHypot * sin(nRad + nRightAngle));
				float nDiff = abs(nNewLeftZ - nNewRightZ);
				if (nDiff < nSmallest || nSmallest < 0)
				{
					nSmallest = nDiff;
					nSmallestRad = nRad;
				}
			}
			if (nSmallest > 0)
			{
				nRotationY = nRotationY - nSmallestRad;
			}
			//do the same but this timne around the Z axis
			nRotationZ = 0;
			nLeftHypot = static_cast<float>(_hypot(nLeftX, nLeftY));
			nRightHypot = static_cast<float>(_hypot(nRightX, nRightY));
			nLeftAngle = static_cast<float>(atan2(nLeftY, nLeftX));
			nRightAngle = static_cast<float>(atan2(nRightY, nRightX));
			nSmallest = -1;
			nSmallestRad = 0;
			for (int h = 0; h < 180; h++)
			{
				float nRad = (h - 90) * D3DX_PI / 180.0f;
				float nNewLeftY = static_cast<float>(nLeftHypot * sin(nRad + nLeftAngle));
				float nNewRightY = static_cast<float>(nRightHypot * sin(nRad + nRightAngle));
				float nDiff = abs(nNewLeftY - nNewRightY);
				if (nDiff < nSmallest || nSmallest < 0)
				{
					nSmallest = nDiff;
					nSmallestRad = nRad;
				}
			}
			if (nSmallest > 0)
			{
				nRotationZ = nRotationZ + nSmallestRad;
			}
		}
	}
}
//--------------------------------------------------------------------------------------------------//
void CFaceViewView::OnDraw(CDC* /*pDC*/)
{
	 
}
//--------------------------------------------------------------------------------------------------//
// CFaceViewView printing
BOOL CFaceViewView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}
//--------------------------------------------------------------------------------------------------//
void CFaceViewView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}
//--------------------------------------------------------------------------------------------------//
void CFaceViewView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

//--------------------------------------------------------------------------------------------------//
// CFaceViewView diagnostics

#ifdef _DEBUG
void CFaceViewView::AssertValid() const
{
	CView::AssertValid();
}
//--------------------------------------------------------------------------------------------------//
void CFaceViewView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
//--------------------------------------------------------------------------------------------------//
CFaceViewDoc* CFaceViewView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFaceViewDoc)));
	return (CFaceViewDoc*)m_pDocument;
}
#endif //_DEBUG
//--------------------------------------------------------------------------------------------------//
void CFaceViewView::Init(CEffects* pEffects, D3DFILLMODE defaultFillmode)
{
	m_Fillmode = defaultFillmode;
	m_pEffects = pEffects;

	CRect R;
	GetClientRect(&R);
	ReCreateBuffers(R.Width(), R.Height());
	
	D3DVIEWPORT9 vp = { 0, 0, 4096, 4096, 0.0f, 1.0f };
	m_pDepthMap = new DrawableTex2D(4096, 4096, 1, D3DFMT_R32F, true, D3DFMT_D24X8, vp, false);
	m_pDepthMap->OnResetDevice(gd3dDevice);
}
//--------------------------------------------------------------------------------------------------//
void CFaceViewView::ReCreateBuffers(int w, int h)
{
	// Destroy the old ones.
	ReleaseCOM(m_SwapChain);
	ReleaseCOM(m_DepthStencil);

	// Create the swapchain associated with this view object.
	m_d3dPP.hDeviceWindow = GetSafeHwnd();
	m_d3dPP.BackBufferWidth = w;
	m_d3dPP.BackBufferHeight = h;
	HRESULT hr = gd3dDevice->CreateAdditionalSwapChain(&m_d3dPP, &m_SwapChain);

	// A swap chain doesn't have an implicit depth/stencil buffer, so
	// we need to create and manage one ourselves.
	hr = gd3dDevice->CreateDepthStencilSurface(
		w, h, D3DFMT_D24S8,
		D3DMULTISAMPLE_NONE, 0,
		true, // discard depth 
		&m_DepthStencil, 0);
}
//--------------------------------------------------------------------------------------------------//
// CFaceViewView message handlers
void CFaceViewView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// Don't try to create buffers if device does not exist or if one of
	// the window dimensions has been resized to zero, as creating a swapchain
	// with zero dimensions fails.
	if (gd3dDevice && cx > 0 && cy > 0)
	{
		ReCreateBuffers(cx, cy);

		float w = (float)cx;
		float h = (float)cy;

		// If window is resized, readjust aspect ratio on perspective view only.
	
	}
}
//--------------------------------------------------------------------------------------------------//
BOOL CFaceViewView::OnEraseBkgnd(CDC* pDC)
{
	// Stop MFC from erasing the background since we do this with D3D.
	// This avoids flicker when resizing the window, for example.
	return FALSE;
}
//--------------------------------------------------------------------------------------------------//
HRESULT CFaceViewView::DrawTexturedQuad(LPDIRECT3DDEVICE9 pDevice,
	FLOAT x, FLOAT y, FLOAT z,
	FLOAT width, FLOAT height,
	D3DXVECTOR2 uvTopLeft, D3DXVECTOR2 uvTopRight,
	D3DXVECTOR2 uvBottomLeft, D3DXVECTOR2 uvBottomRight,
	LPDIRECT3DTEXTURE9 pTexture)
{
	 
	 
	PPVERT QuadCenter[4] =
	{		x,			y,			z, 1,  uvTopLeft.x,		uvTopLeft.y,	 0, 0,
		x + width,	y,				z, 1,  uvBottomRight.x,	uvTopLeft.y,	 0, 0,
		x,			y + height,		z, 1,  uvTopLeft.x,		uvBottomRight.y, 0, 0,
		x + width,	y + height,		z, 1,  uvBottomRight.x,	uvBottomRight.y, 0, 0
	};
	 
	IDirect3DVertexBuffer9* pVBCenter = 0;
	HRESULT hr = gd3dDevice->CreateVertexBuffer(sizeof(PPVERT) ,
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
		0,
		D3DPOOL_DEFAULT,
		&pVBCenter,
		NULL);
	LPVOID pVBData;
	if (SUCCEEDED(pVBCenter->Lock(0, 0, &pVBData, D3DLOCK_DISCARD)))
	{
		CopyMemory(pVBData, QuadCenter, sizeof(QuadCenter) );
		pVBCenter->Unlock();
	}
	 
	hr = m_pEffects->m_pFX->SetTechnique(m_pEffects->m_hDrawTexTech);
	//hr = gd3dDevice->SetVertexDeclaration(VertexSS::Decl);
	gd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	hr = m_pEffects->m_pFX->SetTexture(m_pEffects->m_hTexture, pTexture);
	UINT numPasses = 0;
	hr = m_pEffects->m_pFX->Begin(&numPasses, 0);
	for (UINT k = 0; k < numPasses; ++k)
	{
		hr = m_pEffects->m_pFX->BeginPass(k);
		gd3dDevice->SetStreamSource(0, pVBCenter, 0, sizeof(PPVERT));
		hr = gd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		if (FAILED(hr))
		{
			ATLASSERT(0);
		}
		hr = m_pEffects->m_pFX->EndPass();
	}
	hr = m_pEffects->m_pFX->End();
	pVBCenter->Release();
	//pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
	//return pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(quad[0]));
	return hr;
}
//--------------------------------------------------------------------------------------------------//
HRESULT CFaceViewView::DrawTransformedQuad(LPDIRECT3DDEVICE9 pDevice,
	FLOAT x, FLOAT y, FLOAT z,
	FLOAT width, FLOAT height,
	D3DXVECTOR2 uvTopLeft, D3DXVECTOR2 uvTopRight,
	D3DXVECTOR2 uvBottomLeft, D3DXVECTOR2 uvBottomRight,
	D3DCOLOR c1, D3DCOLOR c2, D3DCOLOR c3, D3DCOLOR c4)
{
	struct
	{
		float pos[4];
		D3DCOLOR color;
		float uv[2];
	} quad[] =
	{
		x,			y,				z, 1.f,		c1, uvTopLeft.x,		uvTopLeft.y,
		x + width,	y,				z, 1.f,		c2, uvBottomRight.x,	uvTopLeft.y,
		x,			y + height,		z, 1.f,		c3, uvTopLeft.x,		uvBottomRight.y,
		x + width,	y + height,		z, 1.f,		c4, uvBottomRight.x,	uvBottomRight.y,
	};
	pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
	return pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(quad[0]));
}
//--------------------------------------------------------------------------------------------------//
HRESULT CFaceViewView::DrawTransformedQuad(LPDIRECT3DDEVICE9 pDevice,
	FLOAT x, FLOAT y, FLOAT z,
	FLOAT width, FLOAT height,
	D3DCOLOR c1,
	D3DCOLOR c2,
	D3DCOLOR c3,
	D3DCOLOR c4)
{
	return DrawTransformedQuad(pDevice, x, y, z, width, height,
		D3DXVECTOR2(0, 0), D3DXVECTOR2(1, 0), D3DXVECTOR2(0, 1), D3DXVECTOR2(1, 1),
		c1, c2, c3, c4);
}