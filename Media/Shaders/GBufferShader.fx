#include "GBuffer.h"

cbuffer VSConstantBuffer : register( b0 )
{
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
    float3 Pos : POSITION;
    float3 Norm : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL0;
};

PS_INPUT VS( VS_INPUT input)
{
    PS_INPUT output;
	
	output.Pos = mul(float4(input.Pos,1),MVP);
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
