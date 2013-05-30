#include "SimplexNoise.fx"
#include "GBuffer.h"
#define CS_GROUP_DIM 18

#define TILE_SIZE 128

#define ROAD_WIDTH 9
#define PAVE_WIDTH 14

#define OVERLAP_SCALE 1.05f

RWTexture2D<float4> albedoTex : register(u0);
RWTexture2D<float4> normalTex : register(u1);
RWTexture2D<float> heightTex : register(u2);

uint2 poorRNG(float2 xy);
float minDist(float2 l1, float2 l2, float2 p);

float getTileCoeff(float2 pos);
bool isLeftOf(float2 a, float2 b, float2 p);
bool isAccepted(float2 pos, float tileInd);
float2 getShiftedCoords(float2 p);


cbuffer CSPass1CSCB : register( b0 )
{
	uint2 bufferDim;
	int2 coords;
	uint tileSize;
}

#define NUM_BIOMES 9
#define MIN_ROAD_BIOME_INDEX 2
#define MAX_ROAD_BIOME_INDEX 6
#define NOISE_ITERATIONS 12

void getHeightNoisesBoundsAccept( in float2 pos, out float terrainHeight, out float noises[NOISE_ITERATIONS], out float2 bounds[4], out bool accept[4]);
	
static const float scales[NOISE_ITERATIONS] = {150000,30000,12800,640,320,
	160,80,40,20,
	10,5,2.5};

static const float coeffs[NUM_BIOMES][NOISE_ITERATIONS] = {
/* {2048,512,128,64,32,16,8,4,2,1,0.5f,0.25f},
{768,376,128,64,32,16,8,4,2,1,0.5f,0.25f},
{512,128,64,32,16,8,4,2,1,0.5f,0.25f,0.125f}, */
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},//
{128,0,0,0,0,0,0,0,0,0,0,0},//
{256,0,0,0,0,0,0,0,0,0,0,0},//
{128,0,0,0,0,0,0,0,0,0,0,0},//
{0,0,0,0,0,0,0,0,0,0,0,0},//
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
};

groupshared float sGroupHeights[CS_GROUP_DIM][CS_GROUP_DIM];

[numthreads(CS_GROUP_DIM, CS_GROUP_DIM, 1)]
void CSPass1(uint3 groupID 			: SV_GroupID,
			uint3 groupThreadID    	: SV_GroupThreadID,
			uint groupIndex 		: SV_GroupIndex)
{
	//Takes into account extra values around the edges
	int2 pixIndex = groupThreadID.xy - 1 + (CS_GROUP_DIM-2) * groupID.xy;
	
	float2 pos = (float2)pixIndex/(bufferDim-1) - 0.50f;
	pos = pos * OVERLAP_SCALE * tileSize + coords;
	
	float terrainheight = 0;
	float noises[NOISE_ITERATIONS];
	float2 bounds[4];
	bool accept[4];

	getHeightNoisesBoundsAccept(pos,terrainheight,noises,bounds,accept);
	
	uint diag = 0;
	
	float roadDist = 9999999.0f;
	
	bool doDiag = 0;

	[loop] for (int i = 0; i < 4; i++) {
	
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
	
	//SpecPower is normalised between -1 and 1
	//Then unpacked to between 0 and 128
	int SpecPower = 128;
	//SpecAmount is normalised between 0 and 1
	//Then unpacked between 0 and 1
	float SpecAmount = 0.1f;
	
	float3 colour = 0;
	float height = 0;
	
	if (roadDist < ROAD_WIDTH) {
		colour = float3(0.55,0.52,0.52);
		height = 5.0f;
	}
	else if (roadDist < PAVE_WIDTH) {
		height = 5.15f;
		colour = float3(0.259,0.259,0.259);
	}
	else {
		colour = float3(0.90,0.90,0.90);
		height = 5.15f;
	}
	
	//Water
	if (terrainheight == 0 && (!accept[0] && !accept[1] && !accept[2] && !accept[3])) {
		colour = float3(0,0,0.5f);
		height = 0.25f*(noises[5] + noises[9] + noises[10]);
		SpecPower = 20;
		SpecAmount = 1;
	}
	
	height+=terrainheight;

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
	
	if (all(groupThreadID.xy > 0) && all(groupThreadID.xy < CS_GROUP_DIM-1)) {
		albedoTex[pixIndex.xy] = float4(colour,SpecAmount);
		normalTex[pixIndex.xy] = float4(normal,SpecPower/64.0f - 1);
		heightTex[pixIndex.xy] = height;
	}
}

//********CITY CS************//

cbuffer CSCityPassCB : register( b0 )
{
	int2 tileCoords;
	uint tileLength;
}

AppendStructuredBuffer<Instance> sInstance : register(u0);

[numthreads(16, 16, 1)]
void CSCityPass(uint3 dispatchID : SV_DispatchThreadID)
{
	float2 p1 = TILE_SIZE * dispatchID.xy - ((float)tileLength/2);
	float2 pos = tileCoords + p1;
	
	float baseheight = (noise2D(pos.x/100,pos.y/72)/2)+0.5f;
	baseheight = 15 + pow(baseheight,2)*(500);
	
	#ifdef CITY_PASS_LOW
	if (baseheight < 50) {
		return;
	}
	#endif
	
	float terrainheight = 0;
	float noises[NOISE_ITERATIONS];
	float2 bounds[4];
	bool accept[4];

	getHeightNoisesBoundsAccept(pos,terrainheight,noises,bounds,accept);
	
	uint numAccept = 0;
	for (uint i = 0; i < 4; i++) {
		numAccept+=(accept[i] && accept[(i+1)%4]) ? 1 : 0;
	}
	
	if (numAccept <= 1) {
		return;
	}
		
	float3 col;
	col.r = (noise2D(pos.x,pos.y)/2)+0.5f;
	col.g = (noise2D(pos.x+900,pos.y-800)/2)+0.5f;
	col.b = pow((noise2D(pos.x-900,pos.y+8000)/2)+0.5f,2);
	
	uint2 rn;
	#if defined(CITY_PASS_LOW) || defined(CITY_PASS_MED)
	rn = 1;
	#else
	rn = poorRNG(pos)%3 + 1;
	#endif
	
	const float maxFootPrint = (TILE_SIZE - (PAVE_WIDTH*2))/2;
	float2 fp = float2(maxFootPrint/rn.x,maxFootPrint/rn.y);
	
	const float2 bl = float2(PAVE_WIDTH,PAVE_WIDTH);

	for (uint i = 0; i < rn.x; i++) {
		for (uint j = 0; j < rn.y; j++) {
			if (i == 1 && j == 1 && rn.x == 3 && rn.y == 3) {
				continue;
			}
			Instance i0;
			
			float3 p;
			p.xz = p1 + bl + 2*fp * float2(i+0.5f,j+0.5f);
			
			p.y = terrainheight-5 + baseheight;
			
			i0.mPos = p;
			i0.mSize = float3(fp.x,baseheight,fp.y);
			i0.mColour = col;
			sInstance.Append(i0);
			
			float height = noise2D(p.x/120,p.z/98);
			baseheight = baseheight + height * 0.3f * baseheight;
		}
	}
}

//*******UTILITY FUNCTIONS**********//

void getHeightNoisesBoundsAccept( in float2 pos, out float terrainheight, out float noises[NOISE_ITERATIONS], out float2 bounds[4], out bool accept[4]) {
	
	[loop] for (int i = 0; i < 12; i++) {
		float2 p2 = pos/(0.25f*scales[i]);
		noises[i] = (noise2D(p2.x,p2.y)/2)+0.5f;
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
	
	float tc = getTileCoeff(pos);
	uint ind0 = floor(tc);
	uint ind1 = ceil(tc);
	float m = smoothstep(ind0,ind1,tc);
	terrainheight = 0;
	for (int i = 0; i < NOISE_ITERATIONS; i++) {
		float c0 = coeffs[ind0][i];
		float c1 = coeffs[ind1][i];
		float mult = lerp(c0,c1,m);
		terrainheight+=noises[i] * mult;
	}
	
	float2 cCo[4];
	cCo[0] = tilePos[2];
	cCo[1] = (pos2.x - tilePos[2].x < 0) ? tilePos[1] : tilePos[3];
	cCo[2] = (pos2.y - tilePos[2].y < 0) ? tilePos[4] : tilePos[0];
	cCo[3] = float2(cCo[1].x,cCo[2].y);
	
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

	bounds[0] = getShiftedCoords(bl);
	bounds[1] = getShiftedCoords(bl + float2(TILE_SIZE,0));
	bounds[2] = getShiftedCoords(bl + float2(TILE_SIZE,TILE_SIZE));
	bounds[3] = getShiftedCoords(bl + float2(0,TILE_SIZE));
	
	
	[unroll] for (int i = 0; i < 4; i++) {
		accept[i] = isAccepted(bounds[i], getTileCoeff(bounds[i]));
	}
	
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
	return p;
	//float rn = (float)(poorRNG(p)%100)/100.0f;
	//return p + TILE_SIZE * pow(rn*0.50f,2);
}

bool isAccepted(float2 pos, float tileInd) {

 	if (tileInd < MIN_ROAD_BIOME_INDEX || tileInd > MAX_ROAD_BIOME_INDEX) {
		return 0;
	}
	
	float2 pos1 = pos/1000;
	float2 pos2 = pos/500;
	float acc = 0;
	acc+=(poorRNG(pos).x%128)/256.0f - 0.4f;
	
	bool accept = (acc>-0.33f) ? 1 : 0;
	return accept;
}

float getTileCoeff(float2 pos) {
	const float scale = 30000;
	float n = (noise2D(pos.x/scale,pos.y/scale)/2)+0.5f;
	return min(NUM_BIOMES-1,NUM_BIOMES*n);
}

uint2 poorRNG(float2 xy)
{
	uint m_z = asuint(asuint(xy.x) - xy.y);
	uint m_w = asuint(asuint(xy.y) + xy.x);
	
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