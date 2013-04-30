#include "SimplexNoise.fx"
#include "GBuffer.h"
#define CS_GROUP_DIM 18

#define TILE_SIZE 128

#define OVERLAP_SCALE 1.05f

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
	
static const float scales[NOISE_ITERATIONS] = {150000,30000,12800,640,320,
	160,80,40,20,
	10,5,2.5};

static const float bases[NOISE_ITERATIONS] = {256,128,64,32,16,8,4,2,1,0.5f,0.25f,0.125f};

static const float coeffs[5][12] = {
{256,128,64,32,16,8,4,2,1,0.5f,0.25f,0.125f},
{256,128,64,32,16,8,4,2,1,0.5f,0.25f,0.125f},
{256,128,64,32,16,8,4,2,1,0.5f,0.25f,0.125f},
{256,128,64,32,16,8,4,2,1,0.5f,0.25f,0.125f},
{0,0,0,0,0,0,0,0,0,0.5f,0.25f,0.125f}
//{256,128,64,32,16,8,4,2,1,0.5f,0.25f,0.125f}
};

groupshared float sGroupHeights[CS_GROUP_DIM][CS_GROUP_DIM];

[numthreads(CS_GROUP_DIM, CS_GROUP_DIM, 1)]
void CSPass1(uint3 groupID 			: SV_GroupID,
			uint3 groupThreadID    	: SV_GroupThreadID,
			uint groupIndex 		: SV_GroupIndex)
{
	//Takes into account extra values around the edges
	int2 pixIndex = groupThreadID.xy - 1 + (CS_GROUP_DIM-2) * groupID;
	
	float2 pos = (float2)pixIndex/(bufferDim-1) - 0.50f;
	pos = pos * OVERLAP_SCALE * tileSize + coords;
	
	float3 colour = 0;
	float height = 0;
	
	float noises[NOISE_ITERATIONS];
	float terrainheight = 0;
	
	[loop] for (int i = 0; i < 12; i++) {
		noises[i] =  noise2D(pos.x/(0.25*scales[i]),pos.y/(0.25*scales[i]));
	}
	
	//Find the bottom left value of the quadrant we're assumed to be in
	int2 tile_id = floor(pos/TILE_SIZE);
	float2 bl = TILE_SIZE * tile_id;
	
	float2 tilePos[5];
	tilePos[0] = bl + float2(0,TILE_SIZE);
	tilePos[1] = bl + float2(-TILE_SIZE,0);
	tilePos[2] = bl;
	tilePos[3] = bl + float2(TILE_SIZE,0);
	tilePos[4] = bl + float2(0,-TILE_SIZE);
	
	float2 pos2 = pos - float2(TILE_SIZE/2,TILE_SIZE/2);
	
	float2 cCo[3];
	cCo[0] = tilePos[2];
	cCo[1] = (pos2.x - tilePos[2].x < 0) ? tilePos[1] : tilePos[3];
	cCo[2] = (pos2.y - tilePos[2].y < 0) ? tilePos[4] : tilePos[0];
	
	float bCo[3];
	
	{
		//Calculate barycentric coordinates
		float2 v23 = cCo[1] - cCo[2];
		float2 vp3 = pos2 - cCo[2];
		float2 v13 = cCo[0] - cCo[2];
		
		float det = (v23.y * v13.x - v23.x * v13.y);
		
		bCo[0] = (v23.y * vp3.x - v23.x * vp3.y)/det;
		bCo[1] = ((-v13.y) * vp3.x + v13.x * vp3.y)/det; 
		bCo[2] = 1 - bCo[0] - bCo[1];
	}
	
	uint baryInds[3];
	
	for (int i = 0; i < 3; i++) {
		const float scale = 30000;
		float n = noise2D(cCo[i].x/scale,cCo[i].y/scale);
		//Mountains
		if (n > 0.8f) {
			baryInds[i] = 0;
		}
		//Rolling hills
		else if (n > 0.6f) {
			baryInds[i] = 1;
		}
		//Mostly Flat nothing
		else if (n > 0.5f) {
			baryInds[i] = 2;
		}
		//Suburbs etc.
		else if (n > 0.2f) {
			baryInds[i] = 3;
		}
		//City, blocks
		else {
			baryInds[i] = 4;
		}
	}
	
	
	for (int i = 0; i < 12; i++) {
		float mult = 0;
		[unroll] for (int j = 0; j < 3; j++) {
			mult+= coeffs[baryInds[j]][i] * bCo[j];
		}
		terrainheight+=noises[i] * mult;
		//terrainheight+=noises[i] * bases[i];
	}
	
	float2 top = getShiftedCoords(tilePos[0]);
	float2 left = getShiftedCoords(tilePos[1]);
	float2 centre = getShiftedCoords(tilePos[2]);
	float2 right = getShiftedCoords(tilePos[3]);
	float2 bottom = getShiftedCoords(tilePos[4]);
	
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
	
	uint diag = 0;
	
	float roadDist = 9999999.0f;
	
	bool doDiag = 1;

	for (int i = 0; i < 4; i++) {
	
		int ind1 = i%4;
		int ind2 = (i+1)%4;
		
		if (accept[ind1]) {
			float dist;
			if (accept[ind2]) {
				dist = minDist(bounds[ind1],bounds[ind2],pos);
				if (diag) {
					dist+=2.0f;
				}
				diag = 0;
			}
			else {
				dist = length(bounds[ind1] - pos);
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
			roadDist = min(roadDist,dist);
		}
		else {
			diag = 0;
		}
	}
	
 	const float roadWidth = 6;
	const float paveWidth = 8;
	const float nearRoadWidth = 25;
	
	if (roadDist < roadWidth) {
		float delta = 0.025f*noises[10]+0.0125*noises[11];
		height = 0.2f*delta;
		colour = float3(0.55,0.52,0.52);
		colour.xyz+=delta;
	}
	else if (roadDist < paveWidth) {
		height = 0.15f;
		colour = float3(0.259,0.259,0.259);
	}
	else if (roadDist < nearRoadWidth) {
		height = 0.15f;
		colour = float3(1,0,0);
	}
	else {
		colour = 1;
		height = 0;
	}
	//colour = (roadDist-paveWidth)/100;
	height+=terrainheight;
	colour = bCo[0];
	//colour.x = baryInds[0]/5.0f;
	//colour.y = baryInds[1]/5.0f;
	//colour.z = baryInds[2]/5.0f;
	
	//float val = (noises[0] + noises[1] + noises[2] + 3)/6;

	//height = blNoise * 1000;
	
	//Calculate normals
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
	
/* 	const float3 vertical = float3(0,1,0);
	float dotprod = dot(vertical,normal);
	
	
	if (dotprod > 0.50f) {
		colour = float3(1.0f,1.0f,1.0f);
		SpecPower = 20;
		SpecAmount = 0.75f;
	}
	else {
		float2 colPos = pos/20.0f;
		float3 colNoise;
		colNoise.x = noise2D(colPos.x,colPos.y);
		colNoise.y = noise2D(colPos.x+1000,colPos.y-1000);
		colNoise.z = noise2D(colPos.x+10000,colPos.y-10000);
		colNoise = (colNoise / 2) + 0.5f;
		
		const float3 rock1 = float3(0.63f,0.59f,0.70f);
		const float3 rock2 = float3(0.70f,0.611f,0.588f);
		
		colour = lerp(rock1,rock2,colNoise);
	} */
	
	if (all(groupThreadID.xy > 0) && all(groupThreadID.xy < CS_GROUP_DIM-1)) {
		albedoTex[pixIndex.xy] = float4(colour,SpecAmount/2);
		normalTex[pixIndex.xy] =  float4(normal,SpecPower/128.0f);
		heightTex[pixIndex.xy] = height;
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
	// float dv = sqrt(pos2.x*pos2.x+pos2.y*pos2.y) + 0.5*noise2D(pos2.x,pos2.y);
	// float sv = abs(sin(dv));
	// if (sv > 0.9) {
		// return 1;
	// }
	float acc = 0;
	//acc += noise2D(pos1.x,pos1.y) + 0.7f*noise2D(pos2.x,pos2.y);
	acc+=(poorRNG(pos).x%128)/256.0f - 0.4f;
	
	bool accept = (acc>-0.3f) ? 1 : 0;
	return accept;
}