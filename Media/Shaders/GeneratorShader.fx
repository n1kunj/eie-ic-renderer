#include "SimplexNoise.fx"
#define CS_GROUP_DIM 16

RWTexture2D<float4> albedoTex : register(t0);
RWTexture2D<float4> normalTex : register(t1);
RWTexture2D<float> heightTex : register(t2);

cbuffer CSPass1CSCB : register( b0 )
{
	uint2 bufferDim;
	int2 coords;
}

[numthreads(CS_GROUP_DIM, CS_GROUP_DIM, 1)]
void CSPass1(uint3 groupId 			: SV_GroupID,
				uint3 dispatchThreadId 	: SV_DispatchThreadID,
				uint3 groupThreadId    	: SV_GroupThreadID,
				uint groupIndex 		: SV_GroupIndex)
{
	float r = (float)dispatchThreadId.x/(bufferDim.x-1) - 0.5f;
	float g = (float)dispatchThreadId.y/(bufferDim.y-1) - 0.5f;
	r = r * 512 + coords.x;
	g = g * 512 + coords.y;
	
	r/=500.0f;
	g/=500.0f;
	float3 col;
	float3 noise;
	noise.x = noise2D(r,g);
	noise.y = noise2D(r+1000.0f,g+1000.0f);
	noise.z = noise2D(r-1000.0f,g-1000.0f);
	col = ((noise/2) + 0.5f);
	
	albedoTex[dispatchThreadId.xy] = float4(col,1.0f);
	normalTex[dispatchThreadId.xy] = float4(0,1,0,0);
	heightTex[dispatchThreadId.xy] = col.x * 1000;
}