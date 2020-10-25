////////////////////////////////////////////////
// Scene variables.
// Simple fx file to render obj files.
// code-section.com

static const float SHADOW_EPSILON = 0.000005f;
static const float SMAP_SIZE = 4096;

float4x4 mWorld	: World;
float4x4 mWVP	: WorldViewProjection;
float4x4 mWIT	: WorldInverseTranspose;
float3 vEye;

uniform extern float  gFarPlane;
 

uniform extern float gBlend = 1;
uniform extern float4x4 gWorld;
uniform extern float4x4 gWorldInvTrans; 
uniform extern float4x4  gLightWVP;
uniform extern float4  gAmbientMtrl;
uniform extern float3  gLightVecW;
uniform extern float2 gProjTexScale = float2(1.0, 1.0);
uniform extern float3  gEyePosW;
uniform extern texture gShadowMap;
uniform extern float4  gDiffuseMtrl; 
uniform extern texture gTexProj;

float3 lightPos = { 1.f, 1.f, 1.f };
float4 lightColor = { 1.f, 1.f, 1.f, 1.f };
float4 sceneAmbient = { 0.15f, 0.15f, 0.15f, 1.f };



// Diffuse options.
float4 diffuseColor = { 0.65f, 0.65f, 0.7f, 1.f };
texture texDiffuse;
sampler tsDiffuse = sampler_state { Texture = (texDiffuse); MipFilter = LINEAR; MinFilter = LINEAR; MagFilter = LINEAR; };
bool useDiffuseTexture = false;


// Specular options.
float4 specularColor = { 1.f, 1.f, 1.f, 1.f };
texture texSpecular;
sampler tsSpecular = sampler_state { Texture = (texSpecular); MipFilter = LINEAR; MinFilter = LINEAR; MagFilter = LINEAR; };
float specularPower = 128.f;
bool useSpecularTexture = false;


// Ambient options.
float4 ambientColor = {0.1f, 0.1f, 0.1f, 1.f };
texture texAmbient;
sampler tsAmbient = sampler_state { Texture = (texAmbient); MipFilter = LINEAR; MinFilter = LINEAR; MagFilter = LINEAR; };
bool useAmbientTexture = false;

sampler ShadowMapS = sampler_state
{
	Texture = <gShadowMap>;
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

sampler TexSProj = sampler_state
{
	Texture = <gTexProj>;
	MinFilter = Anisotropic;
	MaxAnisotropy = 8;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};
 
  

struct OutputVSProj
{
	float4 posH     : POSITION0;
	float3 posW     : TEXCOORD0;
	float3 normalW  : TEXCOORD1;
	float3 toEyeW   : TEXCOORD2;
	float4 projTex  : TEXCOORD3;
};
 



void BuildShadowMapVS(float3 posL : POSITION0,
	out float4 posH : POSITION0,
	out float2 depth : TEXCOORD0)
{
	// Render from light's perspective.
	posH = mul(float4(posL, 1.0f), gLightWVP);

	// Propagate z- and w-coordinates.
	depth = posH.zw;
}

float4 BuildShadowMapPS(float2 depth : TEXCOORD0) : COLOR
{
	// Each pixel in the shadow map stores the pixel depth from the 
	// light source in normalized device coordinates.
	return depth.x / depth.y; // z / w
}

technique BuildShadowMapTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 BuildShadowMapVS();
		pixelShader = compile ps_2_0 BuildShadowMapPS();
	}
}

OutputVSProj ProjTexVS(float3 posL : POSITION0, float3 normalL : NORMAL0)
{
	// Zero out our output.
	OutputVSProj outVS = (OutputVSProj)0;

	// Transform normal to world space.
	outVS.normalW = mul(float4(normalL, 0.0f), mWIT).xyz;//gWorldInvTrans

	// Transform vertex position to world space.
	outVS.posW = mul(float4(posL, 1.0f), mWorld).xyz;//gWorld

	// Compute the unit vector from the vertex to the eye.
	outVS.toEyeW = vEye - outVS.posW; //outVS.toEyeW = gEyePosW - outVS.posW;

	// Transform to homogeneous clip space.
	outVS.posH = mul(float4(posL, 1.0f), mWVP);//gWVP

	// Render from light source to generate projective texture coordinates.
	outVS.projTex = mul(float4(posL, 1.0f), gLightWVP);

	// Done--return the output.
	return outVS;
}

float4 ProjTexPS(float3 posW     : TEXCOORD0,
	float3 normalW : TEXCOORD1,
	float3 toEyeW : TEXCOORD2,
	float4 projTex : TEXCOORD3) : COLOR
{


	// Interpolated normals can become unnormal--so normalize.
	normalW = normalize(normalW);
	toEyeW = normalize(toEyeW);


	float3 ambient = 0.5;// gAmbientMtrl*gLight.ambient;

	projTex.xy = projTex.xy /  projTex.w;

	if (abs(projTex.x) > 1.0f || abs(projTex.y) > 1.0f || projTex.z < 0.0f)
	{
		clip(-1);
	}
	 

	projTex.x = 0.5f * projTex.x + 0.5f;
	projTex.y = -0.5f * projTex.y + 0.5f;

	// Compute pixel depth for shadowing.
	float depth = projTex.z / projTex.w;
	// Transform to texel space
	float2 texelpos = SMAP_SIZE * projTex.xy;

	// Determine the lerp amounts.           
	float2 lerps = frac(texelpos);

	// 2x2 percentage closest filter.
	float dx = 1.0f / SMAP_SIZE;
	float s0 = (tex2D(ShadowMapS, projTex.xy).r + SHADOW_EPSILON < depth) ? 0.5f : 1.0f;
	float s1 = (tex2D(ShadowMapS, projTex.xy + float2(dx, 0.0f)).r + SHADOW_EPSILON < depth) ? 0.5f : 1.0f;
	float s2 = (tex2D(ShadowMapS, projTex.xy + float2(0.0f, dx)).r + SHADOW_EPSILON < depth) ? 0.5f : 1.0f;
	float s3 = (tex2D(ShadowMapS, projTex.xy + float2(dx, dx)).r + SHADOW_EPSILON < depth) ? 0.5f : 1.0f;
	float shadowCoeff = lerp(lerp(s0, s1, lerps.x),
		lerp(s2, s3, lerps.x),
		lerps.y);
	s0 = (tex2D(ShadowMapS, projTex.xy).r + SHADOW_EPSILON < depth) ? 0.5f : 1.0f;
	shadowCoeff = s0;
	 
	float3 l = normalize(gLightVecW);
	float s = dot(l, normalW);
	 
	if (s > 0.5)
	{
	//	clip(-1);
	}

	if (shadowCoeff < 0.51)
	{
		//clip(-1);
		return float4(1.0f, 0.0f, 0.0f, 1.0f);//texColor.a);
	}
	 
	projTex.w = 0;

	projTex.xy = projTex.xy * gProjTexScale;

	float4 texColor = tex2Dlod(TexSProj, projTex);	 
	float3 litColor = (texColor.rgb);// *shadowCoeff;
	return float4(litColor, 1.0f);//texColor.a);
}

technique ProjTexTech
{
	pass P0
	{
		// Specify the vertex and pixel shader associated with this pass.
		vertexShader = compile vs_3_0 ProjTexVS();
		pixelShader = compile ps_3_0 ProjTexPS();
		Cullmode = CCW;
	}
}
 
//-----------------------------------------------------------------------------// 
float4 DrawTexPS( float2 tex0 : TEXCOORD0 ) : COLOR0
{
    return tex2D(TexSProj, tex0).rgba;
}
//-----------------------------------------------------------------------------//
technique DrawTexTech
{
    pass p0
    {
        VertexShader = null;
        PixelShader = compile ps_3_0 DrawTexPS();
        ZEnable = false;
    }
}
OutputVSProj DrawColorVS(float3 posL : POSITION0, float3 normalL : NORMAL0)
{
	// Zero out our output.
	OutputVSProj outVS = (OutputVSProj)0;

	// Transform normal to world space.
	outVS.normalW = mul(float4(normalL, 0.0f), mWIT).xyz;//gWorldInvTrans

	// Transform vertex position to world space.
	outVS.posW = mul(float4(posL, 1.0f), mWorld).xyz;//gWorld

	// Compute the unit vector from the vertex to the eye.
	outVS.toEyeW = vEye - outVS.posW; //outVS.toEyeW = gEyePosW - outVS.posW;

	// Transform to homogeneous clip space.
	outVS.posH = mul(float4(posL, 1.0f), mWVP);//gWVP

	// Render from light source to generate projective texture coordinates.
	outVS.projTex = mul(float4(posL, 1.0f), gLightWVP);

	// Done--return the output.
	return outVS;
}
//-----------------------------------------------------------------------------// 
float4 DrawColorPS( float2 tex0 : TEXCOORD0 ) : COLOR0
{
    return float4(0,0,1,1);//return blue
}
//-----------------------------------------------------------------------------//
technique DrawColorTech
{
    pass p0
    {
        VertexShader = compile vs_3_0 DrawColorVS();
        PixelShader = compile ps_3_0 DrawColorPS();
        ZEnable = false;
    }
}