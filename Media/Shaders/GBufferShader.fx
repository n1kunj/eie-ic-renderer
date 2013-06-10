#include "GBuffer.h"

cbuffer VSConstantBuffer : register( b0 )
{
	matrix cModel;
	matrix cMV;
	matrix cVP;
	int3 cOffset;
}

cbuffer PSConstantBuffer : register( b0 )
{
	float3 cAlbedo;
	float cSpecPower;
	float cSpecAmount;
}

struct VS_INPUT
{
    float3 mPos : POSITION;
    float3 mNorm : NORMAL;
};

struct PS_INPUT
{
    float4 mPos : SV_POSITION;
    float3 mNorm : NORMAL0;
};

struct PS_INPUT_INSTANCED
{
    float4 mPos : SV_POSITION;
	float3 mWorldPos : POSITION0;
    float3 mNorm : NORMAL0;
	float3 mColour : COLOUR0;
};

PS_INPUT VS( VS_INPUT input)
{
    PS_INPUT output;
	
	matrix Model2 = cModel;
	Model2._m30_m31_m32+=cOffset;
	
	float4 pos = mul(float4(input.mPos,1),Model2);
	
	output.mPos = mul(pos,cVP);
	
	output.mNorm = normalize(mul(input.mNorm,(float3x3)cMV)).xyz;
    return output;
}

StructuredBuffer<Instance> bIndices : register(t0);

PS_INPUT_INSTANCED VS_INSTANCED( VS_INPUT input, uint index : SV_InstanceID)
{
    PS_INPUT_INSTANCED output;
	
	Instance i0 = bIndices[index];
	
	float angle = i0.mRotY;
	
	float cosa = cos(angle);
	float sina = sin(angle);
	float3x3 rotmat = {cosa,0,-sina,0,1,0,sina,0,cosa};

	
	float4 pos = float4(input.mPos,1);
	pos.xyz*=i0.mSize;
	
	pos = mul(pos,cModel);
	
	output.mWorldPos = pos.xyz + i0.mPos;
	
	pos.xyz = mul(pos.xyz,rotmat);
	
	pos.xyz+=i0.mPos;
	

	pos.xyz+=cOffset;

	
	output.mPos = mul(pos,cVP);
	output.mNorm = normalize(mul(input.mNorm,(float3x3)cMV)).xyz;
	output.mColour = i0.mColour;
    return output;
}

GBuffer PS( PS_INPUT input )
{
	GBuffer output;
	output.mNormSpec = float4(EncodeSphereMap(input.mNorm),cSpecAmount,cSpecPower);
	output.mAlbedo = float4(cAlbedo,1.0f);
	
	return output;
}

GBuffer PS_INSTANCED( PS_INPUT_INSTANCED input )
{
	GBuffer output;
	//output.mNormSpec = float4(EncodeSphereMap(input.mNorm),0,128);

	output.mNormSpec = float4(EncodeSphereMap(input.mNorm),cSpecAmount,cSpecPower);
	output.mAlbedo = float4(input.mColour,1.0f);
	float3 wp = input.mWorldPos;
	//output.mAlbedo.r = 1.0f;
	
	return output;
}
