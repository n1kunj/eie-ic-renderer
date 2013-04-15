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
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

ConstantOutputType ColorPatchConstantFunction(InputPatch<VS_OUTPUT,3> inputPatch, uint patchId : SV_PrimitiveID)
{    
	ConstantOutputType output;
	float tessAmount = 2;
	// Set the tessellation factors for the three edges of the triangle.
	output.edges[0] = tessAmount;
	output.edges[1] = tessAmount;
	output.edges[2] = tessAmount;
	// Set the tessellation factor for tessallating inside the triangle.
	output.inside = tessAmount;

	return output;
}

struct HS_OUTPUT
{
	float3 Pos : POSITION;
};

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ColorPatchConstantFunction")]
HS_OUTPUT HS(InputPatch<VS_OUTPUT,3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
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

[domain("tri")]
DS_OUTPUT DS(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 3> patch)
{
	DS_OUTPUT output;
	
	float3 vertPos = uvwCoord.x * patch[0].Pos + uvwCoord.y * patch[1].Pos + uvwCoord.z * patch[2].Pos;
	
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
