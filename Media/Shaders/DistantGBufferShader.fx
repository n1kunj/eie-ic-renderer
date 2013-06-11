#include "GBuffer.h"

struct VS_INPUT
{
	float3 mPos : POSITION;
};

struct VS_OUTPUT
{
	float3 mPos : POSITION;
};

VS_OUTPUT VS( VS_INPUT input)
{
	VS_OUTPUT output;
	
	output.mPos = input.mPos;

	return output;
}

struct HS_C_OUTPUT
{
	float mEdges[4] : SV_TessFactor;
	float mInside[2] : SV_InsideTessFactor;
};

HS_C_OUTPUT HS_CONSTANT(InputPatch<VS_OUTPUT,4> inputPatch, uint patchId : SV_PrimitiveID)
{    
	HS_C_OUTPUT output;
	float tessAmount = 4;
	
	output.mEdges[0] = tessAmount;
	output.mEdges[1] = tessAmount;
	output.mEdges[2] = tessAmount;
	output.mEdges[3] = tessAmount;
	
	output.mInside[0] = tessAmount;
	output.mInside[1] = tessAmount;

	return output;
}

struct HS_OUTPUT
{
	float3 mPos : POSITION;
};

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HS_CONSTANT")]
HS_OUTPUT HS(InputPatch<VS_OUTPUT,4> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	HS_OUTPUT output;
	output.mPos = patch[pointId].mPos;
	return output;
}

struct DS_OUTPUT
{
	float4 mPos : SV_POSITION;
	float2 mUV : TEXCOORDS;
};

cbuffer DSConstantBuffer : register( b0 )
{
	matrix cModel;
	matrix cView;
	matrix cVP;
	matrix cMV;
	matrix cMVP;
	int3 cOffset;
}

SamplerState sDefault : register(s0);
SamplerState sAnisotropic : register(s1);

Texture2D<float> tHeight : register(t0);

[domain("quad")]
DS_OUTPUT DS(HS_C_OUTPUT input, float2 coord : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 4> patch)
{
	DS_OUTPUT output;
	
	float3 pos1 = lerp(patch[0].mPos,patch[1].mPos,coord.y);
	float3 pos2 = lerp(patch[3].mPos,patch[2].mPos,coord.y);
	float3 vertPos = lerp(pos1,pos2,coord.x);
	
	float2 uvs = vertPos.xz + float2(0.5f,0.5f);
	
	float height = tHeight.SampleLevel(sDefault,uvs,0);
	
	matrix Model2 = cModel;
	Model2._m30_m31_m32+=cOffset;
	
	vertPos = mul(float4(vertPos,1.0f),Model2).xyz;
	
	vertPos.y += height;
	
	output.mPos = mul(float4(vertPos,1.0f),cVP);
	
	output.mUV = uvs;
	
	return output;
}

Texture2D<float4> tAlbedo : register(t0);
Texture2D<float4> tNormal : register(t1);

GBuffer PS( DS_OUTPUT input )
{
	GBuffer output;
	float4 texmap = tAlbedo.Sample(sAnisotropic,float2(input.mUV));
	float4 normalmap = tNormal.Sample(sAnisotropic,float2(input.mUV));
	
	float3 albedo = texmap.xyz;

	float3 normal = normalmap.xyz;
	float specAmount = texmap.w;
	float specPower = 64 * (normalmap.w + 1); //Format is packed between -1 and 1, need to unpack it here

	normal = normalize(mul(normal,(float3x3)cMV));

	output.mAlbedo = float4(albedo,1.0f);
	output.mNormSpec = float4(EncodeSphereMap(normal),specAmount,specPower);
	output.mEmittance = float4(0,0,0,1);
	
	return output;
}
