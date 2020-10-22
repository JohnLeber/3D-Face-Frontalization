#pragma once


//===============================================================
struct VertexSS
{
	VertexSS()
		:pos(0.0f, 0.0f, 0.0f, 0.0f),
		tex0(0.0f, 0.0f), tex1(0.0f, 0.0f) {}
	VertexSS(float x, float y, float z, float w,
		float u, float v, float u2, float v2) :pos(x, y, z, w), tex0(u, v), tex1(u2, v2) {}
	VertexSS(const D3DXVECTOR3& v, const D3DXVECTOR2& uv)
		:pos(v), tex0(uv), tex1(uv) {}

	D3DXVECTOR4 pos;
	D3DXVECTOR2 tex0;
	D3DXVECTOR2 tex1;

	static IDirect3DVertexDeclaration9* Decl;
};
//===============================================================
struct VertexPT
{
	VertexPT()
		:pos(0.0f, 0.0f, 0.0f),
		tex0(0.0f, 0.0f) {}
	VertexPT(float x, float y, float z,
		float u, float v) :pos(x, y, z), tex0(u, v) {}
	VertexPT(const D3DXVECTOR3& v, const D3DXVECTOR2& uv)
		:pos(v), tex0(uv) {}

	D3DXVECTOR3 pos;
	D3DXVECTOR2 tex0;

	static IDirect3DVertexDeclaration9* Decl;
};
//===============================================================
struct VertexPNT
{
	VertexPNT()
		:pos(0.0f, 0.0f, 0.0f),
		normal(0.0f, 0.0f, 0.0f),
		tex0(0.0f, 0.0f)
		//tex1(0.0f, 0.0f),
		//tangent(0.0f, 0.0f, 0.0f),
		/*binormal(0.0f, 0.0f, 0.0f)*/ {}
	VertexPNT(float x, float y, float z,
		float nx, float ny, float nz,
		float u, float v) :pos(x, y, z), normal(nx, ny, nz), tex0(u, v)/*,  tex1(u2,v2)*/ {}
	VertexPNT(const D3DXVECTOR3& v, const D3DXVECTOR3& n, const D3DXVECTOR2& uv, const D3DXVECTOR2& uv2)
		:pos(v), normal(n), tex0(uv)/*, tex1(uv2)*/ {}

	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 tex0;
	//D3DXVECTOR2 tex1;//test

	//D3DXVECTOR3 tangent;//NM TEST
	//D3DXVECTOR3 binormal;//NM TEST

	static IDirect3DVertexDeclaration9* Decl;
};
// Call in constructor and destructor, respectively, of derived application class.
void InitAllVertexDeclarations(IDirect3DDevice9* pD3DDevice);
void DestroyAllVertexDeclarations();