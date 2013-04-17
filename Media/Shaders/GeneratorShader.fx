#include "SimplexNoise.fx"
#include "GBuffer.h"
#define CS_GROUP_DIM 16
//mother fucker
#define normalise normalize

#define EPSILON 0.01f

RWTexture2D<float4> albedoTex : register(t0);
RWTexture2D<float2> normalTex : register(t1);
RWTexture2D<float> heightTex : register(t2);

cbuffer CSPass1CSCB : register( b0 )
{
	uint2 bufferDim;
	int2 coords;
	uint tileSize;
}

float heightFunc(float2 pos) {
//return 0;
	float height = 0;
	float base = 500.0f;
	float scale = 3000.0f;
	for (int i = 0; i < 8; i++) {
		height+=base * noise2D(pos.x/scale,pos.y/scale);
		base/=2;
		scale/=2;
	}

	return height;
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
	
	float2 hpos = float2(r,g);
	float height = heightFunc(hpos);
	
 	float hp[4];
	hp[0] = heightFunc(hpos+float2(-EPSILON,0));
	hp[1] = heightFunc(hpos+float2(EPSILON,0));
	hp[2] = heightFunc(hpos+float2(0,-EPSILON));
	hp[3] = heightFunc(hpos+float2(-EPSILON,0));
	
	float3 va = normalize(float3(2*EPSILON,hp[1]-hp[0],0));
	float3 vb = normalize(float3(0,hp[3]-hp[2],2*EPSILON));
	float3 normal = normalize(cross(vb,va));

	//normal = float3(0,1,0);
	
	r/=2000.0f;
	g/=2000.0f;
	float3 noise;
	noise.x = noise2D(r,g);
	noise.y = noise2D(r+1000.0f,g+1000.0f);
	noise.z = noise2D(r-1000.0f,g-1000.0f);
	float3 colour = ((noise/2) + 0.5f);
	
	//normal = (normal / 2) + 0.5f;
	//normal = normalise(float3(0.1f,0.1f,0.1f));

	albedoTex[dispatchThreadId.xy] = float4(colour,1.0f);
	normalTex[dispatchThreadId.xy] = EncodeSphereMap(normal);
	heightTex[dispatchThreadId.xy] = height;
}