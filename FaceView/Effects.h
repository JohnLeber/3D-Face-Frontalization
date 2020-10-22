#pragma once
class CEffects
{
 
public:
	CEffects();
	~CEffects();
	bool Initialize();

	ID3DXEffect* m_pFX;
	D3DXHANDLE m_hTexture, m_hDrawTexTech, m_hProjTech, m_hDepthTech, m_hColorTech;
	D3DXHANDLE m_hShadowMap;
	D3DXHANDLE m_hLightWVP, m_hFarPlane;


};

