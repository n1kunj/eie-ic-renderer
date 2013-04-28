#include "SimplexNoise.fx"
#include "GBuffer.h"
#define CS_GROUP_DIM 18

#define TILE_SIZE 100

RWTexture2D<float4> albedoTex : register(t0);
RWTexture2D<float4> normalTex : register(t1);
RWTexture2D<float> heightTex : register(t2);

uint2 poorRNG(float2 xy);

float minDist(float2 l1, float2 l2, float2 p);
bool isLeftOf(float2 a, float2 b, float2 p);
bool isAccepted(float2 pos);

float2 getShiftedCoords(float2 p);

cbuffer CSPass1CSCB : register( b0 )
{
	uint2 bufferDim;
	int2 coords;
	uint tileSize;
}

#define NOISE_ITERATIONS 12

// static const float scales[NOISE_ITERATIONS] = {5120,2560,1280,640,320,
	// 160,80,40,20,
	// 10,5,2.5};
	
static const float scales[NOISE_ITERATIONS] = {150000,30000,1280,640,320,
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
	float terrainheight = 0;
	[loop] for (int i = 0; i < 12; i++) {
		//noises[i] =  noise2D(2 * hpos.x,2 * hpos.y);
		noises[i] =  noise2D(hpos.x/(0.25*scales[i]),hpos.y/(0.25*scales[i]));
		//height+=(((int)(noises[i] * bases[i]))/4)*4;
	}
	//height = (((int)height)/5)*5;
	//height*=20.0f;
	for (int i = 0; i < 2; i++) {
		terrainheight+=noises[i] * bases[i];
		//height+=noises[i] * bases[i];
	}
	terrainheight/=2.0f;
	
	float2 pos = (hpos);
	
	//Find the bottom left value of the quadrant we're assumed to be in
	int2 tile_id = floor(pos/TILE_SIZE);
	float2 bl = TILE_SIZE * tile_id;
	
	float2 centre = getShiftedCoords(bl);
	float2 top = getShiftedCoords(bl + float2(0,TILE_SIZE));
	float2 right = getShiftedCoords(bl + float2(TILE_SIZE,0));
	float2 bottom = getShiftedCoords(bl + float2(0,-TILE_SIZE));
	float2 left = getShiftedCoords(bl + float2(-TILE_SIZE,0));
	
	bool lct = isLeftOf(centre,top,pos);
	bool lcr = isLeftOf(centre,right,pos);
	bool lcb = isLeftOf(centre,bottom,pos);
	bool lcl = isLeftOf(centre,left,pos);
	
	if (!lcl && lct) {
		tile_id += int2(-1,0);
	}
	else if (!lcb && lcl) {
		tile_id += int2(-1,-1);
	}
	else if (!lcr && lcb) {
		tile_id += int2(0,-1);
	}
	bl = TILE_SIZE * tile_id;

	float2 bounds[4];
	
	bounds[0] = getShiftedCoords(bl);
	bounds[1] = getShiftedCoords(bl + float2(TILE_SIZE,0));
	bounds[2] = getShiftedCoords(bl + float2(TILE_SIZE,TILE_SIZE));
	bounds[3] = getShiftedCoords(bl + float2(0,TILE_SIZE));
	
	bool accept[4];
	accept[0] = isAccepted(bounds[0]);
	accept[1] = isAccepted(bounds[1]);
	accept[2] = isAccepted(bounds[2]);
	accept[3] = isAccepted(bounds[3]);
	
	uint hasRoad = 0;
	uint hasPave = 0;
	uint nearRoad = 0;
	uint numRoads = 0;
	uint diag = 0;
	uint filler = 0;
	
	float roadWidth = 9;
	float paveWidth = 13;
	float nearRoadWidth = 25;
	
	bool doDiag = 1;

	for (int i = 0; i < 4; i++) {
	
		int ind0 = (i-1)%4;
		int ind1 = i%4;
		int ind2 = (i+1)%4;
	
		float dist = minDist(bounds[ind1],bounds[ind2],pos);
	
		if (accept[ind1]) {
			if (accept[ind2]) {
				if (!(!diag || (diag && isLeftOf(bounds[ind1],bounds[ind2],pos)))){
					dist+=5.0f;
				}
				hasRoad = (dist < roadWidth) ? 1 : hasRoad;
				hasPave = (dist < paveWidth) ? 1 : hasPave;
				nearRoad = (dist < nearRoadWidth) ? 1 : nearRoad;

				numRoads++;
				diag = 0;
			}
			else {
				float df = length(bounds[ind1] - pos);
				filler = (df < roadWidth) ? 1 : filler;
				nearRoad = (df < nearRoadWidth) ? 1 : nearRoad;
				hasPave = (df < paveWidth) ? 1 : hasPave;
				if (doDiag) {
					if (!diag) {
					bounds[ind2] = bounds[ind1];
					accept[ind2] = accept[ind1];
					diag = 1;
					}
					else {
						diag = 0;
					}
				}
			}
		}
		else {
			diag = 0;
		}
	}
	
	float2 noisepos = bounds[0]/10.0f;
	//float2 noisepos = bounds[0]/7350.2f;

	
	float hval = pow((noise2D(noisepos.x,noisepos.y)+1)/2,2);
	
	float3 colour;
	
	if (hasRoad || filler) {
		float delta = 0.025f*noises[10]+0.0125*noises[11];
		height = 0.2f*delta;
		colour = float3(0.55,0.52,0.52);
		colour.xyz+=delta;
		//colour.y+=0.05f*noises[10];
		//colour.z+=0.05f*noises[9];
	}
	else if (hasPave) {
		height = 0.15f;
		colour = float3(0.259,0.259,0.259);
	}
	else if (nearRoad) {
		height = 0.15f;
		colour = float3(1,0,0);
	
	}
	else if (!numRoads) {
		height = 1;
		colour = float3(0,1,0);
	
	}
	else if (numRoads < 2) {
		//height = 0.2f;
		//colour = 1;
		height = 1;
		colour = float3(0,1,0);
	}
	else {
		colour = 1;
		height = 0;
		//if (tileSize >512) {
			//height = 30;
			//height = 10 + 200 * hval;
		//}
	}
	height+=terrainheight;
	height = 0;
	//height = 0;
	//colour = 1;
	
	//colour = float3(height,height,height);
	//colour = 5*height;

/* 	int TILE_SIZE = 128;
	
	float2 loc = TILE_SIZE*floor(hpos/TILE_SIZE);
	
	float mindist2 = 999999999999.0f;
	float mindist = 999999999999.0f;
	
	[unroll] for (int i = -1; i < 2; i++) {
		[unroll] for (int j = -1; j < 2; j++) {
			float2 loca = loc + TILE_SIZE * int2(i,j);
			//float2 loca = loc;
			uint2 rand = poorRNG(loca);
			float2 loc2 = loca + rand%TILE_SIZE;
			float2 distxy = loc2 - pos;
			distxy*=distxy;
			//float dist = abs(distxy.x)+abs(distxy.y);
			float dist = distxy.x + distxy.y;
			[flatten] if (dist < mindist) {
				mindist2 = mindist;
				mindist = dist;
			}
			else if (dist < mindist2) {
				mindist2 = dist;
			}
		}
	}
	
	uint2 random = poorRNG(loc);
	
	float2 loc2 = loc + random%TILE_SIZE;
	
	float2 distxy = loc2 - pos;
	distxy*=distxy;
	float dist = sqrt(distxy.x + distxy.y);
	//float dist = abs(distxy.x)+abs(distxy.y);
	// float maxdist = sqrt(TILE_SIZE*TILE_SIZE*2);
	// mindist2 = sqrt(mindist2);
	// mindist = sqrt(mindist);
	
	float maxdist = sqrt(TILE_SIZE*TILE_SIZE*2);
	mindist2 = (mindist2);
	mindist = (mindist);
	
	//float maxdist = 4 * TILE_SIZE;
	height = sqrt(mindist2 - mindist)/maxdist;
	//height = dist/maxdist;
	
/* 	pos.y/=10;
	float val = 15 + 5*noises[1];
	pos.x/=val;
	
	float v1 = (1 + sin(pos.x))/2;
	float v2 = (1 + sin(pos.y))/2;
	
	v1 = (v1 > val/100.0f) ? 1 : 0;
	v2 = (v2 > 0.10f) ? 1 : 0;

	//height = (v1 + v2)/2;
	height = v1;
	
	height = (height > 0.05f) ? 1 : 0;

	*/
		
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
	
	//height = hp[0]+hp[1]+hp[2]+hp[3] + 4*height;
	//height/=8;
	//normal = float3(0,1,0);

	
	//SpecPower is normalised between -1 and 1
	//Then unpacked to between -128 and 128
	int SpecPower = 128;
	//SpecAmount is normalised between 0 and 1
	//Then unpacked between 0 and 2
	float SpecAmount = 0.1f;

	
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
	
/* 	if (dotprod > 0.50f && height > 500) {
		colour = float3(1.0f,1.0f,1.0f);
		//normal = lerp(vertical,normal,0.5f);
		SpecPower = 20;
		SpecAmount = 0.75f;
	}
	else {
		colour = lerp(rock1,rock2,colNoise);
	} */
	
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

uint2 poorRNG(float2 xy)
{
	uint m_z = asuint(asuint(xy.x) - xy.y);
	uint m_w = asuint(asuint(xy.y) + xy.x);

	//uint m_z = asuint(xy.x - xy.y);
	//uint m_w = asuint(xy.y + xy.x);
	
	//m_z = ((m_z >> 16) ^ m_z) * 0x45d9f3b;
	//m_w = ((m_w >> 16) ^ m_w) * 0x45d9f3b;
	
    //m_z = ((m_z >> 16) ^ m_w) * 0x45d9f3b;
    //m_z = ((m_z >> 16) ^ m_z);
	
    m_w = ((m_z >> 16) ^ m_w) * 0x45d9f3b;
    m_w = ((m_w >> 16) ^ m_w);
	
	m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
	
	uint r1 = (m_z << 16) + m_w;
	
	m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
	
	uint r2 = (m_z << 16) + m_w;
	
	return uint2(r1,r2);
}

float minDist( float2 l1, float2 l2, float2 p) {
	float2 vec = l2-l1;
	float lensq = vec.x*vec.x + vec.y*vec.y;
	
	float t = dot(p-l1,vec)/lensq;
	
	float2 proj;
	
	[flatten] if (t < 0.0f) {
		proj = l1;
	}
	else if (t >1.0f) {
		proj = l2;
	}
	else {
		proj = l1 + t * vec;
	}
	return length(p-proj);
}

//Return +1 if left of, 0 if on or to the right of
//Results assume a is "below" b
bool isLeftOf(float2 a, float2 b, float2 p) {
	float2 AB = b-a;
	float2 AP = p-b;
	return clamp(sign(AB.x*AP.y-AB.y*AP.x),0,1);
	
}

float2 getShiftedCoords(float2 p) {
	//return p;
	float rn = (float)(poorRNG(p)%100)/100.0f;
	return p + TILE_SIZE * pow(rn*0.50f,2);
}

bool isAccepted(float2 pos) {
	float2 pos1 = pos/1000;
	float2 pos2 = pos/500;
	float dv = sqrt(pos2.x*pos2.x+pos2.y*pos2.y) + 0.5*noise2D(pos2.x,pos2.y);
	float sv = abs(sin(dv));
	if (sv > 0.9) {
		return 1;
	}
	float acc = 0;
	acc += noise2D(pos1.x,pos1.y) + 0.7f*noise2D(pos2.x,pos2.y);
	acc+=(poorRNG(pos).x%128)/256.0f - 0.4f;
	
	bool accept = (acc>-0.2f) ? 1 : 0;
	return accept;
}