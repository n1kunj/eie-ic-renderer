//--------------------------------------------------------------------------------------
// File: Tutorial04.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix Model;
	matrix View;
	matrix Projection;
	matrix MVP;
}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( float4 Pos : POSITION, float4 Color : COLOR)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
	
	//output.Pos = mul(Model,Pos);
	//output.Pos = mul(View, output.Pos);
	//output.Pos = mul(Projection, output.Pos);
	
	//matrix MVM = mul(View, Model);
	//matrix MVPM = mul(Projection, MVM);
	//output.Pos = mul(MVPM,Pos);
	
	output.Pos = mul(MVP,Pos);
	
    output.Color = Color;
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
	return input.Color;
}
