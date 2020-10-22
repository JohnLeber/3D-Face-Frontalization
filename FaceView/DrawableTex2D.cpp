#include "pch.h"
#include "DrawableTex2D.h"

//=============================================================================
// DrawableTex2D.cpp by Frank Luna (C) 2005 All Rights Reserved.
//=============================================================================

//------------------------------------------------------------------------------//
DrawableTex2D::DrawableTex2D(UINT width, UINT height, UINT mipLevels,
	D3DFORMAT texFormat, bool useDepthBuffer,
	D3DFORMAT depthFormat, D3DVIEWPORT9& viewport, bool autoGenMips)
	: mTex(0), mRTS(0), mTopSurf(0), mWidth(width), mHeight(height),
	mMipLevels(mipLevels), mTexFormat(texFormat), mUseDepthBuffer(useDepthBuffer),
	mDepthFormat(depthFormat), mViewPort(viewport), mAutoGenMips(autoGenMips)
{

}
//------------------------------------------------------------------------------//
DrawableTex2D::~DrawableTex2D()
{
	OnLostDevice();
}
//------------------------------------------------------------------------------//
IDirect3DTexture9* DrawableTex2D::d3dTex()
{
	return mTex;
}
//------------------------------------------------------------------------------//
void DrawableTex2D::OnLostDevice()
{
	if (mTopSurf)
	{
		UINT t = mTopSurf->Release();
		mTopSurf = 0;
	}
	if (mRTS)
	{
		UINT t = mRTS->Release();
		mRTS = 0;
	}
	if (mTex)
	{
		UINT t = mTex->Release();
		mTex = 0;
	}
}
//------------------------------------------------------------------------------//
void DrawableTex2D::OnResetDevice(IDirect3DDevice9* pDevice)
{
	UINT usage = D3DUSAGE_RENDERTARGET;
	if (mAutoGenMips)
	{
		usage |= D3DUSAGE_AUTOGENMIPMAP;
	}

	HRESULT hr = D3DXCreateTexture(pDevice, mWidth, mHeight, mMipLevels, usage, mTexFormat, D3DPOOL_DEFAULT, &mTex);
	if (SUCCEEDED(hr))
	{
		hr = D3DXCreateRenderToSurface(pDevice, mWidth, mHeight, mTexFormat, mUseDepthBuffer, mDepthFormat, &mRTS);
		hr = mTex->GetSurfaceLevel(0, &mTopSurf);
	}
	else
	{
	}
}
//------------------------------------------------------------------------------//
void DrawableTex2D::beginScene()
{
	if (mRTS == 0)
	{
		return;
	}
	HRESULT hr = mRTS->BeginScene(mTopSurf, &mViewPort);
	if (FAILED(hr))
	{
	}
	//IDirect3DDevice9* pD3DDevice = 0;
	//mRTS->GetDevice(&pD3DDevice);
	//hr = pD3DDevice->Clear(0, 0, D3DCLEAR_TARGET| D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0, 0);
}
//------------------------------------------------------------------------------//
void DrawableTex2D::endScene()
{
	if (mRTS == 0)
	{
		return;
	}
	HRESULT hr = mRTS->EndScene(D3DX_FILTER_NONE);
}
//------------------------------------------------------------------------------//
