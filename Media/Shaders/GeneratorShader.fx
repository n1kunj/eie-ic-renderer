#include "SimplexNoise.fx"
#include "GBuffer.h"
#define CS_GROUP_DIM 16
//mother fucker
#define normalise normalize

#define EPSILON 0.1f

RWTexture2D<float4> albedoTex : register(t0);
RWTexture2D<float4> normalTex : register(t1);
RWTexture2D<float> heightTex : register(t2);

cbuffer CSPass1CSCB : register( b0 )
{
	uint2 bufferDim;
	int2 coords;
	uint tileSize;
}

static const float scales[10] = {3000,1500,750,
	375,187.5,93.75,46.875,23.4375,
	11.71875,5.859375};

static const float bases[10] = {500,250,125,
	62.5,31.25,15.625,
	7.8125,3.90625,1.953125,0.9765625};

float heightFunc(float2 pos) {
//return 0;
	float height = 0;
	for (int i = 0; i < 10; i++) {
		height+=bases[i] * noise2D(pos.x/scales[i],pos.y/scales[i]);
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
	
	//SpecPower is normalised between -1 and 1
	//Then unpacked to between -128 and 128
	int SpecPower = 128;
	//SpecAmount is normalised between 0 and 1
	//Then unpacked between 0 and 2
	float SpecAmount = 0.1f;

	
	const float3 vertical = float3(0,1,0);
	float dotprod = dot(vertical,normal);
	
	float3 colour;
	
	float3 rock1 = float3(0.63f,0.59f,0.70f);
	float3 rock2 = float3(0.70f,0.611f,0.588f);
	
	//float3 rock1 = float3(0.55f,0.55f,0.55f);
	//float3 rock2 = float3(0.70f,0.70f,0.70f);
	
	//float3 colour = float3(0.63f,0.59f,0.70f);
	//colour = float3(1,0,0);
	float2 colPos = hpos/20.0f;
	float3 colNoise;
	colNoise.x = noise2D(colPos.x,colPos.y);
	colNoise.y = noise2D(colPos.x+1000,colPos.y-1000);
	colNoise.z = noise2D(colPos.x+10000,colPos.y-10000);
	colNoise = (colNoise / 2) + 0.5f;

	
	if (dotprod > 0.65f) {
		colour = float3(1.0f,1.0f,1.0f);
		normal = lerp(vertical,normal,0.5f);
		
		//Remove the high frequency noises
		for (int i = 8; i < 10; i++) {
				height -= bases[8] * noise2D(hpos.x/scales[8],hpos.y/scales[8]);
		}
		//normal = float3(0,1,0);
		SpecPower = 20;
		SpecAmount = 0.75f;
	}
	else {
		//colour = rock1;
		colour = lerp(rock1,rock2,colNoise);
	}
	

	albedoTex[dispatchThreadId.xy] = float4(colour,SpecAmount/2);
	normalTex[dispatchThreadId.xy] =  float4(normal,SpecPower/128.0f);
	heightTex[dispatchThreadId.xy] = height;
}