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
	
	float4 pos = float4(input.mPos,1);
	pos.xyz*=i0.mSize;
	
	pos = mul(pos,cModel);
	
	pos.xyz+=i0.mPos;
	
	output.mWorldPos = pos.xyz;

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

float filterwidth(float2 v)
{
  float2 fw = max(abs(ddx(v)), abs(ddy(v)));
  return max(fw.x, fw.y);
}

float2 effect(float2 x) {
	if (abs(x.x%256) > 128) {
		return float2(0,0);
	}
	else {
		return float2(1,1);
	}
	//return floor((x)/2) + 2.f * max(((x)/2) - floor((x)/2) - .5f, 0.f);
}

float checker(float2 uv)
{
  float width = filterwidth(uv);
  float2 p0 = uv - .5 * width, p1 = uv + .5 * width;
  #define BUMPINT(x) \
         (floor((x)/2) + 2.f * max(((x)/2) - floor((x)/2) - .5f, 0.f))
  float2 i = (BUMPINT(p1) - BUMPINT(p0)) / width;
  //float2 i = (effect(p1) - effect(p0)) / width;
  return i.x * i.y + (1 - i.x) * (1 - i.y);
}

GBuffer PS_INSTANCED( PS_INPUT_INSTANCED input )
{
	GBuffer output;
	output.mNormSpec = float4(EncodeSphereMap(input.mNorm),cSpecAmount,cSpecPower);
	output.mAlbedo = float4(input.mColour,1.0f);
	float3 wp = input.mWorldPos;
	float check = checker(wp.xy/20);
	output.mAlbedo.rgb *= check;
	
	return output;
}
