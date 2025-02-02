#include "tanspace.fxi"
#include "lighting_common.fxi"

uniform texture2D Color;

///////////////////////////////////////////////////////////////////////////////

uniform sampler2D ColorSampler = sampler_state
{
	Texture = <Color>;
    MagFilter = Linear;
    MinFilter = Anisotropic;
    MipFilter = Linear;
	MaxAnisotropy = 4.0f;
};

///////////////////////////////////////////////////////////////////////////////

struct VertexNrmTex
{
    float3 Position             : POSITION;
    float2 Uv0                  : TEXCOORD0;
    float3 Normal               : NORMAL;
    float4 Color				: COLOR0;
};

struct VertexNrmTex2
{
    float3 Position             : POSITION;
    float2 Uv0                  : TEXCOORD0;
    float2 Uv1                  : TEXCOORD1;
    float3 Normal               : NORMAL;
    float4 Color				: COLOR0;
};

struct VertexNrmTexSkinned
{
    float3 Position             : POSITION;
    float2 Uv0                  : TEXCOORD0;
    float3 Normal               : NORMAL;
    int4   BoneIndices			: BLENDINDICES;
    float4 BoneWeights			: BLENDWEIGHT;
};

struct Fragment
{
    float4 ClipPos		: Position;
    float4 Color		: Color;
    float2 Uv0			: TEXCOORD0;
    float2 Uv1			: TEXCOORD1;
    float4 ClipUserPos	: TEXCOORD2;
};

///////////////////////////////////////////////////////////////////////////////

Fragment vs_lambert(VertexNrmTex VtxIn)
{
	float3 ObjectPos = VtxIn.Position;
	float3 ObjectNrm = VtxIn.Normal;
	float4 WorldPos = mul(float4(ObjectPos.xyz, 1.0f), World);
	float3 WorldNrm = mul(float4(ObjectNrm.xyz, 0.0f), World).xyz;
	float3 ViewNrm = mul(float4(WorldNrm.xyz, 0.0f), ViewInverseTranspose).xyz;

	float3 LightCol = DiffuseLight( WorldPos, WorldNrm );

	Fragment FragOut;

    FragOut.ClipPos = mul(WorldPos, ViewProjection);
	FragOut.Color = float4(ModColor.xyz*LightCol,1.0f);
	FragOut.Uv0 = VtxIn.Uv0;
	FragOut.Uv1 = VtxIn.Uv0;
	FragOut.ClipUserPos = FragOut.ClipPos;

	return FragOut;
}

Fragment vs_lambert_vtxlit(VertexNrmTex VtxIn)
{
	float3 ObjectPos = VtxIn.Position;
	float3 ObjectNrm = VtxIn.Normal;
	float4 WorldPos = mul(float4(ObjectPos.xyz, 1.0f), World);
	float3 WorldNrm = mul(float4(ObjectNrm.xyz, 0.0f), World).xyz;
	float3 ViewNrm = mul(float4(WorldNrm.xyz, 0.0f), ViewInverseTranspose).xyz;

	float3 LightCol = ProcPreVtxLitColor(VtxIn.Color.xyz); 

	Fragment FragOut;

    FragOut.ClipPos = mul(WorldPos, ViewProjection);
	FragOut.Color = float4(ModColor.xyz*LightCol,1.0f);
	FragOut.Uv0 = VtxIn.Uv0;
	FragOut.Uv1 = VtxIn.Uv0;
	FragOut.ClipUserPos = FragOut.ClipPos;

	return FragOut;
}

Fragment vs_lambert_lightpreview(VertexNrmTex VtxIn)
{
	float3 ObjectPos = VtxIn.Position;
	float3 ObjectNrm = VtxIn.Normal;
	float4 WorldPos = mul(float4(ObjectPos.xyz, 1.0f), World);
	float3 WorldNrm = mul(float4(ObjectNrm.xyz, 0.0f), World).xyz;
	float3 ViewNrm = mul(float4(WorldNrm.xyz, 0.0f), ViewInverseTranspose).xyz; 

	float headlight = 0.5f + 0.5f*saturate(dot(ViewNrm, float3(0.0f, 0.0f, -1.0f)));
	float3 dlight = DiffuseLight( WorldPos.xyz, WorldNrm.xyz );
	dlight = PreviewLightSaturate(dlight);
	Fragment FragOut;

    FragOut.ClipPos = mul(WorldPos, ViewProjection);
	FragOut.Color = float4(dlight,1.0f);
	FragOut.Uv0 = VtxIn.Uv0;
	FragOut.Uv1 = VtxIn.Uv0;
	FragOut.ClipUserPos = FragOut.ClipPos;

	return FragOut;
}


Fragment vs_lambert_lightmapped(VertexNrmTex2 VtxIn)
{
	float3 ObjectPos = VtxIn.Position;
	float3 ObjectNrm = VtxIn.Normal;
	float4 WorldPos = mul(float4(ObjectPos.xyz, 1.0f), World);
	float3 WorldNrm = mul(float4(ObjectNrm.xyz, 0.0f), World).xyz;
	float3 ViewNrm = mul(float4(WorldNrm.xyz, 0.0f), ViewInverseTranspose).xyz; 
	float headlight = 0.5f + 0.5f*saturate(dot(ViewNrm, float3(0.0f, 0.0f, -1.0f)));

	Fragment FragOut;

    FragOut.ClipPos = mul(WorldPos, ViewProjection);
	FragOut.Color = float4(ModColor.xyz*headlight,1.0f);
	FragOut.Uv0 = VtxIn.Uv0;
	FragOut.Uv1 = VtxIn.Uv1*float2(1.0f,-1.0f);
	FragOut.ClipUserPos = FragOut.ClipPos;

	return FragOut;
}

Fragment vs_lambert_skinned(VertexNrmTexSkinned VtxIn)
{
	float3 ObjectPos = SkinPosition4(VtxIn.BoneIndices, VtxIn.BoneWeights, VtxIn.Position);
	float3 ObjectNrm = SkinNormal4(VtxIn.BoneIndices, VtxIn.BoneWeights, VtxIn.Normal);

	float4 WorldPos = mul(float4(ObjectPos.xyz, 1.0f), World);
	float3 WorldNrm = mul(float4(ObjectNrm.xyz, 0.0f), World).xyz;

	float3 LightCol = DiffuseLight( WorldPos, WorldNrm );

	Fragment FragOut;

    FragOut.ClipPos = mul(WorldPos, ViewProjection);
	FragOut.Color = float4(LightCol, 1.0f);
	FragOut.Uv0 = VtxIn.Uv0;
	FragOut.Uv1 = VtxIn.Uv0;
	FragOut.ClipUserPos = FragOut.ClipPos;

	return FragOut;
}

///////////////////////////////////////////////////////////////////////////////

float4 ps_modcolor(Fragment FragIn) : COLOR
{
    float4 PixOut;
    PixOut = ModColor;
    return PixOut;
}

float4 ps_vtxcolor(Fragment FragIn) : COLOR
{
    float4 PixOut = float4(FragIn.Color.xyz, 1.0f);
    return PixOut;
}

float4 ps_lambert(Fragment FragIn) : COLOR
{
    float4 PixOut;
   	float4 tex1 = tex2D(ColorSampler, FragIn.Uv0).xyzw;
	PixOut = tex1.xyzw;//*FragIn.Color.xyzw;
	return PixOut;
}

float4 ps_lambert_lightmapped(Fragment FragIn) : COLOR
{
    float4 PixOut;

   	float4 colr = tex2D(ColorSampler, FragIn.Uv0).xyzw;
   	float3 lmap = GetLightmap(FragIn.Uv1);

	PixOut = float4(colr.xyz*lmap.xyz,colr.w*FragIn.Color.w);
	return PixOut;
}

///////////////////////////////////////////////////////////////////////////////

technique main
{
    pass p0
    {
        VertexShader = compile vs_3_0 vs_lambert();
        PixelShader = compile ps_3_0 ps_lambert();
        SrcBlend = SRCALPHA;
        DestBlend = INVSRCALPHA;
		AlphaBlendEnable = true;
		AlphaTestEnable = true;
		ZWriteEnable = true;
		ZFunc = LESS;
		ZEnable = TRUE;
	}
};

technique mainLightMapped
{
    pass p0
    {
        VertexShader = compile vs_3_0 vs_lambert_lightmapped();
        PixelShader = compile ps_3_0 ps_lambert_lightmapped();
        SrcBlend = SRCALPHA;
        DestBlend = INVSRCALPHA;
		AlphaBlendEnable = true;
		AlphaTestEnable = true;
		ZWriteEnable = true;
		ZFunc = LESS;
		ZEnable = TRUE;
	}
};

technique mainVertexLit
{
    pass p0
    {
        VertexShader = compile vs_3_0 vs_lambert_vtxlit();
        PixelShader = compile ps_3_0 ps_lambert();
        SrcBlend = SRCALPHA;
        DestBlend = INVSRCALPHA;
		AlphaBlendEnable = true;
		AlphaTestEnable = true;
		ZWriteEnable = true;
		ZFunc = LESS;
		ZEnable = TRUE;
	}
};

technique mainLightPreview
{
    pass p0
    {
        VertexShader = compile vs_3_0 vs_lambert_lightpreview();
        PixelShader = compile ps_3_0 ps_lambert();
        SrcBlend = SRCALPHA;
        DestBlend = INVSRCALPHA;
		AlphaBlendEnable = true;
		AlphaTestEnable = true;
		ZWriteEnable = true;
		ZFunc = LESS;
		ZEnable = TRUE;
	}
};

technique noZwrite
{
    pass p0
    {
        VertexShader = compile vs_3_0 vs_lambert();
        PixelShader = compile ps_3_0 ps_lambert();
        SrcBlend = SRCALPHA;
        DestBlend = INVSRCALPHA;
		AlphaBlendEnable = true;
		AlphaTestEnable = true;
		ZWriteEnable = false;
	}
};

technique mainSkinned
{
    pass p0
    {
        VertexShader = compile vs_3_0 vs_lambert_skinned();
        PixelShader = compile ps_3_0 ps_lambert();
        SrcBlend = SRCALPHA;
        DestBlend = INVSRCALPHA;
		AlphaBlendEnable = true;
		AlphaTestEnable = true;
		ZWriteEnable = true;
		ZFunc = LESS;
		ZEnable = TRUE;
	}
};

#include "pick.fxi"
