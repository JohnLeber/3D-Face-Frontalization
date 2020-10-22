#include "pch.h" 
#include "vertex.h" 

//// Initialize static variables.
//IDirect3DVertexDeclaration9* VertexPos::Decl = 0;
////=============================================================================
//// Vertex.cpp by Frank Luna (C) 2005 All Rights Reserved.
////=============================================================================
//
//#include "VertexPos.h"
//#include "d3dUtil.h"

  

// Initialize static variables. 
IDirect3DVertexDeclaration9* VertexSS::Decl = 0;
IDirect3DVertexDeclaration9* VertexPT::Decl = 0;
IDirect3DVertexDeclaration9* VertexPNT::Decl = 0;


void InitAllVertexDeclarations(IDirect3DDevice9* pD3DDevice)
{
	 
	// Vertex declaration for post-processing
	const D3DVERTEXELEMENT9 VertexSSElements[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
		{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  1 },
		D3DDECL_END()
	};
	HRESULT hr = pD3DDevice->CreateVertexDeclaration(VertexSSElements, &VertexSS::Decl);

	// VertexPT

	D3DVERTEXELEMENT9 VertexPTElements[] =
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};
	hr = pD3DDevice->CreateVertexDeclaration(VertexPTElements, &VertexPT::Decl);

	//===============================================================
	// VertexPNT

	D3DVERTEXELEMENT9 VertexPNTElements[] =
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		//{0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},//test
		//{0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},//NM TEST
		//{0, 44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0},//NM TEST
		D3DDECL_END()
	};
	hr = pD3DDevice->CreateVertexDeclaration(VertexPNTElements, &VertexPNT::Decl);

 
}
//----------------------------------------------------------------------------------------//
void DestroyAllVertexDeclarations()
{ 
	if (VertexPT::Decl)  VertexPT::Decl->Release();
	if (VertexPNT::Decl)  VertexPNT::Decl->Release(); 
	if (VertexSS::Decl)  VertexSS::Decl->Release();
}
 