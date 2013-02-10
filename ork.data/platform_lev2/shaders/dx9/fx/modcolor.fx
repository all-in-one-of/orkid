///////////////////////////////////////////////////////////////////////////////
// Orkid
// Copyright 1996-2010, Michael T. Mayers
///////////////////////////////////////////////////////////////////////////////

float4x4 mvp;

///////////////////////////////////////////////////////////////////////////////

struct Vertex
{
	float3 Position		: POSITION;
	float4 Color		: COLOR;
	//float4 Normal_S		: NORMAL;
	//float4 Binormal_T	: BINORMAL;
};

///////////////////////////////////////////////////////////////////////////////

struct Fragment
{
	float4 ClipPos : Position;
	float4 Color   : Color;
};

///////////////////////////////////////////////////////////////////////////////

Fragment VSModColor( Vertex VtxIn )
{
	Fragment FragOut;
	
	FragOut.ClipPos = mul( float4( VtxIn.Position, 1.0f ), mvp );
	FragOut.Color = VtxIn.Color;
	return FragOut;
}

///////////////////////////////////////////////////////////////////////////////

float4 PSModColor( Fragment FragIn ) : COLOR
{
	float4 PixOut;
	PixOut = FragIn.Color;
	return PixOut;
}

///////////////////////////////////////////////////////////////////////////////

technique modcolor
{
	pass p0
	{
		VertexShader = compile vs_2_0 VSModColor();
		PixelShader = compile ps_2_0 PSModColor();
		
		//CullMode = NONE; // NONE CW CCW
	}
};

///////////////////////////////////////////////////////////////////////////////