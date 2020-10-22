#pragma once
#pragma once
#ifndef _NODIRECTX
#include <d3dx9.h>
 
class DrawableTex2D
{
public:
	DrawableTex2D(UINT width, UINT height, UINT mipLevels,
		D3DFORMAT texFormat, bool useDepthBuffer,
		D3DFORMAT depthFormat, D3DVIEWPORT9& viewport, bool autoGenMips);
	~DrawableTex2D();

	IDirect3DTexture9* d3dTex();

	void beginScene();
	void endScene();

	void OnLostDevice();
	void OnResetDevice(IDirect3DDevice9* pDevice);
	IDirect3DSurface9* mTopSurf;
private:
	// This class is not designed to be copied.
	DrawableTex2D(const DrawableTex2D& rhs);
	DrawableTex2D& operator=(const DrawableTex2D& rhs);

private:
	IDirect3DTexture9* mTex;
	ID3DXRenderToSurface* mRTS; 
	UINT         mWidth;
	UINT         mHeight;
	UINT         mMipLevels;
	D3DFORMAT    mTexFormat;
	bool         mUseDepthBuffer;
	D3DFORMAT    mDepthFormat;
	D3DVIEWPORT9 mViewPort;
	bool         mAutoGenMips;
};

#endif

