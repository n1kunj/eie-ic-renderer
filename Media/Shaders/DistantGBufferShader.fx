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
	float tessAmount = 2;
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
	float2 UV : TEXCOORDS;
};

cbuffer DSConstantBuffer : register( b0 )
{
	matrix Model;
	matrix VP;
	matrix MV;
	matrix MVP;
	int3 gCoords;
}

Texture2D<float4> albedoTex : register(t0);
SamplerState defaultSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

[domain("quad")]
DS_OUTPUT DS(ConstantOutputType input, float2 coord : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 4> patch)
{
	DS_OUTPUT output;
	
	float3 pos1 = lerp(patch[0].Pos,patch[1].Pos,coord.y);
	float3 pos2 = lerp(patch[3].Pos,patch[2].Pos,coord.y);
	float3 vertPos = lerp(pos1,pos2,coord.x);
	
	float2 uvs = vertPos.xz + float2(0.5f,0.5f);
	//float height = albedoTex.Gather(defaultSampler,uvs);
	//float height = 1000 * albedoTex.Load(uint3(0,0,0));
	//float height = 0;
	
	vertPos = mul(float4(vertPos,1.0f),Model);
	uint repeatval = 100;
	
	//vertPos.y += height;
	
	output.Pos = mul(float4(vertPos,1.0f),VP);

	//output.Pos = mul(float4(vertPos,1.0f),MVP);
	
	float3 normal = float3(0,1,0);

	//output.Norm = normalize(mul(normal,MV));
	output.Norm = normal;
	
	output.UV = uvs;
	
	return output;
}

cbuffer PSConstantBuffer : register( b1 )
{
	float3 Albedo;
	float SpecPower;
	float SpecAmount;
}

GBuffer PS( DS_OUTPUT input )
{
	GBuffer output;
	float4 texmap = albedoTex.Sample(defaultSampler,float2(input.UV));
	output.albedo = texmap;
	//output.albedo = float4(Albedo,1.0f);
	float3 normal = input.Norm;
	//normal = float3(0,1,0);
	//normal = texmap;

	normal = normalize(mul(normal,MV));
	
	output.normal_specular = float4(EncodeSphereMap(normal),SpecAmount,SpecPower);

	
	return output;
}
