#include "SimplexNoise.fx"
#include "GBuffer.h"
#define CS_GROUP_DIM 18

RWTexture2D<float4> albedoTex : register(t0);
RWTexture2D<float4> normalTex : register(t1);
RWTexture2D<float> heightTex : register(t2);

cbuffer CSPass1CSCB : register( b0 )
{
	uint2 bufferDim;
	int2 coords;
	uint tileSize;
}

#define NOISE_ITERATIONS 12

static const float scales[NOISE_ITERATIONS] = {5120,2560,1280,640,320,
	160,80,40,20,
	10,5,2.5};

static const float bases[NOISE_ITERATIONS] = {512,256,128,64,32,16,8,4,2,1,0.5f,0.25f};

groupshared float sGroupHeights[CS_GROUP_DIM][CS_GROUP_DIM];

[numthreads(CS_GROUP_DIM, CS_GROUP_DIM, 1)]
void CSPass1(uint3 groupID 			: SV_GroupID,
			uint3 groupThreadID    	: SV_GroupThreadID,
			uint groupIndex 		: SV_GroupIndex)
{
	//Takes into account extra values around the edges
	int2 pixLoc = groupThreadID.xy - 1 + (CS_GROUP_DIM-2) * groupID;

	float r = (float)pixLoc.x/(bufferDim.x-1) - 0.5f;
	float g = (float)pixLoc.y/(bufferDim.y-1) - 0.5f;
	r = r * tileSize + coords.x;
	g = g * tileSize + coords.y;
	
	float2 hpos = float2(r,g);
	
	
	float noises[NOISE_ITERATIONS];
	float height = 0;
	[loop] for (int i = 0; i < 12; i++) {
		//noises[i] =  noise2D(2 * hpos.x,2 * hpos.y);
		noises[i] =  noise2D(hpos.x/(0.25*scales[i]),hpos.y/(0.25*scales[i]));
		height+=noises[i] * bases[i];
		//height+=(((int)(noises[i] * bases[i]))/4)*4;
	}
	height/=2.0f;
	//height = (((int)height)/10)*10;
	//height*=20.0f;
	height = 0;
	
	for (int i = 0; i < NOISE_ITERATIONS; i++) {
		//height+=noises[i] * bases[i];
	}
	
	sGroupHeights[groupThreadID.x][groupThreadID.y] = height;
	
	GroupMemoryBarrierWithGroupSync();
	
	float hp[4];
	hp[0] = sGroupHeights[groupThreadID.x-1][groupThreadID.y];
	hp[1] = sGroupHeights[groupThreadID.x+1][groupThreadID.y];
	hp[2] = sGroupHeights[groupThreadID.x][groupThreadID.y-1];
	hp[3] = sGroupHeights[groupThreadID.x][groupThreadID.y+1];
	
	float epsilon = (float)tileSize/(bufferDim.x-1);
	float3 va = normalize(float3(2*epsilon,hp[1]-hp[0],0));
	float3 vb = normalize(float3(0,hp[3]-hp[2],2*epsilon));	

	float3 normal = normalize(cross(vb,va));
	
	//SpecPower is normalised between -1 and 1
	//Then unpacked to between -128 and 128
	int SpecPower = 128;
	//SpecAmount is normalised between 0 and 1
	//Then unpacked between 0 and 2
	float SpecAmount = 0.1f;

	//normal = float3(0,1,0);
	
	const float3 vertical = float3(0,1,0);
	float dotprod = dot(vertical,normal);
		
	float3 rock1 = float3(0.63f,0.59f,0.70f);
	float3 rock2 = float3(0.70f,0.611f,0.588f);
	
	float2 colPos = hpos/20.0f;
	float3 colNoise;
	colNoise.x = noise2D(colPos.x,colPos.y);
	colNoise.y = noise2D(colPos.x+1000,colPos.y-1000);
	colNoise.z = noise2D(colPos.x+10000,colPos.y-10000);
	colNoise = (colNoise / 2) + 0.5f;

	float3 colour;
	
	if (dotprod > 0.50f && height > 0) {
		colour = float3(1.0f,1.0f,1.0f);
		//normal = lerp(vertical,normal,0.5f);
		SpecPower = 20;
		SpecAmount = 0.75f;
	}
	else {
		colour = lerp(rock1,rock2,colNoise);
	}
	
	// [flatten] if ((groupThreadID.x + groupThreadID.y)%2==0) {
		// colour = float3(1,0,0);
	// }
	// else {
		// colour = float3(0,1,0);
	// }
	
	if (all(groupThreadID.xy > 0) && all(groupThreadID.xy < CS_GROUP_DIM-1)) {
		albedoTex[pixLoc.xy] = float4(colour,SpecAmount/2);
		normalTex[pixLoc.xy] =  float4(normal,SpecPower/128.0f);
		heightTex[pixLoc.xy] = height;
	}
}
