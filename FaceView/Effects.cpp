#include "pch.h"
#include "Effects.h"
//-----------------------------------------------------------------------------------------//
CEffects::CEffects()
{
	m_pFX = 0;
	m_hTexture = 0;
	m_hDrawTexTech = 0;
	m_hShadowMap = 0;
	m_hLightWVP = 0;
	m_hFarPlane = 0;
	m_hProjTech = 0;
	m_hColorTech = 0;
}
//-----------------------------------------------------------------------------------------//
CEffects::~CEffects()
{
	if (m_pFX)
	{
		m_pFX->Release();
		m_pFX = 0;
	}
}
//-----------------------------------------------------------------------------------------//
bool CEffects::Initialize()
{
	CString strFileName;
	TCHAR szFilePath[MAX_PATH + _ATL_QUOTES_SPACE];
	DWORD dwFLen = ::GetModuleFileName(NULL, szFilePath + 0, MAX_PATH);
	if (dwFLen == 0 || dwFLen == MAX_PATH)
	{
		ATLASSERT(0); 
		return false;
	}
	else
	{
		strFileName = CString(szFilePath);
		long nRight = strFileName.ReverseFind(_T('\\'));
		strFileName = strFileName.Left(nRight);
		strFileName.Append(_T("\\FaceView.fx"));
	}

	LPD3DXBUFFER pErrors = NULL;
	HRESULT hr = D3DXCreateEffectFromFile(gd3dDevice, strFileName, NULL, NULL, 0, NULL, &m_pFX, &pErrors);
	if (FAILED(hr))
	{
		if (pErrors)
		{
			CStringA strErrorsA =  (LPCSTR)pErrors->GetBufferPointer();
			CA2W wErrors = strErrorsA;
			CString strErrors = wErrors;
			MessageBox(NULL, strErrors, L"", MB_OK);
			pErrors->Release();
		}
		return false;
	}

	/*g_hProjTech = m_pFX->GetTechniqueByName("ProjTexTech");
	m_hDepthTech = m_pFX->GetTechniqueByName("BuildShadowMapTech");*/
	m_hDrawTexTech = m_pFX->GetTechniqueByName("DrawTexTech");
	m_hProjTech = m_pFX->GetTechniqueByName("ProjTexTech");
	m_hColorTech = m_pFX->GetTechniqueByName("DrawColorTech");
	m_hDepthTech = m_pFX->GetTechniqueByName("BuildShadowMapTech");
	m_hTexture = m_pFX->GetParameterByName(0, "gTexProj"); 
	m_hShadowMap = m_pFX->GetParameterByName(0, "gShadowMap");
	m_hLightWVP = m_pFX->GetParameterByName(0, "gLightWVP");
	m_hFarPlane = m_pFX->GetParameterByName(0, "gFarPlane");

	return true;
}
//-----------------------------------------------------------------------------------------//