#include "GBuffer.h"

struct VS_INPUT
{
	float4 Pos : POSITION;
};

struct VS_OUTPUT
{
	float3 Pos : POSITION;
};

VS_OUTPUT VS( VS_INPUT input)
{
	VS_OUTPUT output;
	
	output.Pos.xyz = input.Pos;

	return output;
}

struct ConstantOutputType
{
	float edges[4] : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

ConstantOutputType HS_CONSTANT(InputPatch<VS_OUTPUT,4> inputPatch, uint patchId : SV_PrimitiveID)
{    
	ConstantOutputType output;
	float tessAmount = 4;
	// Set the tessellation factors for the three edges of the triangle.
	output.edges[0] = tessAmount;
	output.edges[1] = tessAmount;
	output.edges[2] = tessAmount;
	output.edges[3] = tessAmount;
	// Set the tessellation factor for tessallating inside the triangle.
	output.inside[0] = tessAmount;
	output.inside[1] = tessAmount;

	return output;
}

struct HS_OUTPUT
{
	float3 Pos : POSITION;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HS_CONSTANT")]
HS_OUTPUT HS(InputPatch<VS_OUTPUT,4> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	HS_OUTPUT output;
	output.Pos = patch[pointId].Pos;
	return output;
}

struct DS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 Norm : NORMAL;
};

cbuffer DSConstantBuffer : register( b0 )
{
	matrix MV;
	matrix MVP;
}

[domain("quad")]
DS_OUTPUT DS(ConstantOutputType input, float2 coord : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 4> patch)
{
	DS_OUTPUT output;
	
	float3 pos1 = lerp(patch[0].Pos,patch[1].Pos,coord.y);
	float3 pos2 = lerp(patch[3].Pos,patch[2].Pos,coord.y);
	float3 vertPos = lerp(pos1,pos2,coord.x);	
	
	output.Pos = mul(float4(vertPos,1.0f),MVP);

	output.Norm = normalize(mul(float3(0.0f,1.0f,0.0f),MV));
	
	return output;
}

cbuffer PSConstantBuffer : register( b0 )
{
	float3 Albedo;
	float SpecPower;
	float SpecAmount;
}

GBuffer PS( DS_OUTPUT input )
{
	GBuffer output;
	output.normal_specular = float4(EncodeSphereMap(input.Norm),SpecAmount,SpecPower);
	output.albedo = float4(Albedo,1.0f);
	
	return output;
}
