////////////////////////////////////////////////
// Scene variables.
// Simple fx file to render obj files.
// code-section.com

float4x4 mWorld	: World;
float4x4 mWVP	: WorldViewProjection;
float4x4 mWIT	: WorldInverseTranspose;
float3 vEye;

uniform extern float gBlend = 1;
uniform extern float4x4 gWorld;
uniform extern float4x4 gWorldInvTrans;
uniform extern float4x4 gWVP;
uniform extern float4x4  gLightWVP;
uniform extern float4  gAmbientMtrl;
uniform extern float3  gLightVecW;
uniform extern float2 gProjTexScale = float2(1.0, 1.0);



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





struct TInputVertex
{
	float4 pos			: POSITION;		// Position of this vertex
	float4 normal		: NORMAL;		// Position of the other vertex at the other end of the line.
	float2 tex0			: TEXCOORD0;	// Tex coords
};


struct TOutputVertex
{
	float4 hPos			: POSITION;
	float2 tex0			: TEXCOORD0;
	float3 L			: TEXCOORD1;
	float3 N			: TEXCOORD2;
	float3 V			: TEXCOORD3;
};




TOutputVertex mainVS( TInputVertex IN )
{
	TOutputVertex OUT = (TOutputVertex)0;
	
	float3 worldPos = mul( IN.pos, mWorld ).xyz;
	
	float3 V = normalize(vEye - worldPos);
	OUT.L = V;
	OUT.V = V;
	OUT.N = mul( IN.normal, mWIT );
	OUT.tex0 = IN.tex0;
	OUT.hPos = mul( IN.pos, mWVP );
	
	return OUT;
}



float4 mainPS( TOutputVertex IN, uniform bool bUseDiffuseTexture ) : COLOR
{
	float3 Ln = normalize( IN.L );
	float3 Nn = normalize( IN.N );
	float3 Vn = normalize( IN.V );
	float3 Hn = normalize( Vn + Ln );
	
	float4 finalColor = diffuseColor * dot(Nn,Ln) + ambientColor + sceneAmbient;
	
	if( bUseDiffuseTexture )
		finalColor *= tex2D( tsDiffuse, IN.tex0 );
		
	finalColor += specularColor * pow( dot(Nn,Hn), specularPower );
	return finalColor;
}


technique Textured
{
	pass p0
	{
		//cullmode = none;
		VertexShader = compile vs_2_0 mainVS();
		PixelShader = compile ps_2_0 mainPS( useDiffuseTexture );
	}
}
 
struct OutputVSProj
{
	float4 posH     : POSITION0;
	float3 posW     : TEXCOORD0;
	float3 normalW  : TEXCOORD1;
	float3 toEyeW   : TEXCOORD2;
	float4 projTex  : TEXCOORD3;
};


OutputVSProj ProjTexVS(float3 posL : POSITION0, float3 normalL : NORMAL0)
{
	// Zero out our output.
	OutputVSProj outVS = (OutputVSProj)0;

	// Transform normal to world space.
	outVS.normalW = mul(float4(normalL, 0.0f), gWorldInvTrans).xyz;

	// Transform vertex position to world space.
	outVS.posW = mul(float4(posL, 1.0f), gWorld).xyz;

	// Compute the unit vector from the vertex to the eye.
	outVS.toEyeW = gEyePosW - outVS.posW;

	// Transform to homogeneous clip space.
	outVS.posH = mul(float4(posL, 1.0f), gWVP);

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

	projTex.xy /= projTex.w;

	if (abs(projTex.x) > 1.0f || abs(projTex.y) > 1.0f || projTex.z < 0.0f)
	{
		clip(-1);
	}
	 

	projTex.x = 0.5f * projTex.x + 0.5f;
	projTex.y = -0.5f * projTex.y + 0.5f;

	// Compute pixel depth for shadowing.
	float depth = projTex.z / projTex.w;
	float s0 = (tex2D(ShadowMapS, projTex.xy).r + SHADOW_EPSILON < depth) ? 0.5f : 1.0f;
	float shadowCoeff = s0;

	float3 l = normalize(gLightVecW);
	float s = dot(l, normalW);
	 
	if (s > 0.5)// && fc < 0.8)
	{
		clip(-1);
	}
	 

	if (shadowCoeff < 0.51)
	{
		clip(-1);
	}
	projTex.w = 0;

	projTex.xy = projTex.xy * gProjTexScale;

	float4 texColor = tex2Dlod(TexSProj, projTex);


	//texColor.r = texColor.r + 0.4;
	float3 litColor = (texColor.rgb);// * shadowCoeff;

	// Output the color and the alpha.
	return float4(litColor, gDiffuseMtrl.a * gBlend);//texColor.a);
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
