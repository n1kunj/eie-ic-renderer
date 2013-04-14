#include "GBuffer.h"

cbuffer VSConstantBuffer : register( b0 )
{
	matrix Model;
	matrix View;
	matrix Projection;
	matrix MV;
	matrix MVP;
}

cbuffer PSConstantBuffer : register( b0 )
{
	float3 Albedo;
	float SpecPower;
	float SpecAmount;
}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float3 ModelPos : POSITION0;
    float3 Norm : NORMAL0;
};

PS_INPUT VS( VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
	
	output.Pos = mul(input.Pos,MVP);
	output.ModelPos = mul(input.Pos,MV);
    output.Norm = normalize(mul(input.Norm,MV));
    return output;
}

GBuffer PS( PS_INPUT input )
{
	GBuffer output;
	output.normal_specular = float4(EncodeSphereMap(input.Norm),SpecAmount,SpecPower);
	output.albedo = float4(Albedo,1.0f);
	
	return output;
}
