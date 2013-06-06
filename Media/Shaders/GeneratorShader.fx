#include "SimplexNoise.fx"
#include "GBuffer.h"
#define CS_GROUP_DIM 18

#define TILE_SIZE 128

#define ROAD_WIDTH 9
#define PAVE_WIDTH 14

#define OVERLAP_SCALE 1.05f

#define MAX_BUILDING_TERRAIN_HEIGHT 1000

RWTexture2D<float4> albedoTex : register(u0);
RWTexture2D<float4> normalTex : register(u1);
RWTexture2D<float> heightTex : register(u2);

Texture2D<float3> baseTex : register(t1);
Texture2D<float3> heightTexMap : register(t2);
Texture2D<float3> roadMap : register(t3);
Texture2D<float3> cityMap : register(t4);
SamplerState sDefault : register(s0);

struct terrainData {
float3 mBaseTex;
float3 mHeightTex;
float3 mRoadMap;
float3 mCityMap;
};

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
#define NOISE_ITERATIONS 12

terrainData loadTexMaps(in float2 pos);

void getNoisesBoundsAccept( in float2 pos, in terrainData td, out float noises[NOISE_ITERATIONS], out float2 bounds[4], out bool accept[4]);

void getTerrainInfo(in float2 pos, in terrainData td, in float noises[NOISE_ITERATIONS], in bool accept[4], out float tileCoeff, out float terrainHeight, out float4 tileCols, out float2 tileSpec);

static const float scales[NOISE_ITERATIONS] = {150000,30000,12800,640,320,
	160,80,40,20,
	10,5,2.5};

static const float coeffs[NUM_BIOMES][NOISE_ITERATIONS] = {
/* {2048,512,128,64,32,16,8,4,2,1,0.5f,0.25f},
{768,376,128,64,32,16,8,4,2,1,0.5f,0.25f},
{512,128,64,32,16,8,4,2,1,0.5f,0.25f,0.125f}, */
{0,0,0,0,0,0.25,0,0,0,0.25,0.25,0},
{0,0,0,0,0,0.25,0,0,0,0.25,0.25,0},
{64,0,0,0,0,0,0,0,0,0,0,0},//
{128,0,0,0,0,0,0,0,0,0,0,0},//
{256,0,0,0,0,0,0,0,0,0,0,0},//
{128,0,0,0,0,0,0,0,0,0,0,0},//
{64,0,0,0,0,0,0,0,0,0,0,0},//
{0,0,0,0,0,0.25,0,0,0,0.25,0.25,0},
{0,0,0,0,0,0.25,0,0,0,0.25,0.25,0},
};

#define CITY_COEFF 0

//r,g,b,city
static const float4 colours[NUM_BIOMES] = {
float4(0,0,0.5,1),
float4(0,0,0.5,1),
float4(0.9,0.9,0.9,0),
float4(0.9,0.9,0.9,0),
float4(0.9,0.9,0.9,0),
float4(0.9,0.9,0.9,0),
float4(0.9,0.9,0.9,0),
float4(0,0,0.5,1),
float4(0,0,0.5,1),
};

static const float2 specPowAmount[NUM_BIOMES] = {
float2(20,1),
float2(20,1),
float2(128,0.1f),
float2(128,0.1f),
float2(128,0.1f),
float2(128,0.1f),
float2(128,0.1f),
float2(20,1),
float2(20,1),
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
	
	terrainData tData = loadTexMaps(pos);

	float noises[NOISE_ITERATIONS];
	float2 bounds[4];
	bool accept[4];

	getNoisesBoundsAccept(pos,tData,noises,bounds,accept);
	
	float terrainHeight;
	float tileCoeff;
	float4 tileCols;
	float2 tileSpec;

	getTerrainInfo(pos, tData, noises, accept, tileCoeff, terrainHeight, tileCols, tileSpec);
	
	uint numAccept = 0;
	for (uint i = 0; i < 4; i++) {
		numAccept+=accept[i] ? 1 : 0;
	}
	
	float3 colour = 0;
	float height = 0;
	
	if (numAccept && tileSize <= 4096 && terrainHeight < MAX_BUILDING_TERRAIN_HEIGHT) {
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
		
		if (roadDist < ROAD_WIDTH) {
			colour = float3(0.55,0.52,0.52);
			height = 5.0f;
		}
		else if (roadDist < PAVE_WIDTH) {
			height = 5.15f;
			colour = float3(0.259,0.259,0.259);
		}
		else {
			colour = tileCols.rgb;
			height = 5.15f;
		}
	}
	else {
		colour = tileCols.rgb;
	}
	
	//SpecPower is normalised between -1 and 1
	//Then unpacked to between 0 and 128
	int SpecPower = tileSpec.x;
	//SpecAmount is normalised between 0 and 1
	//Then unpacked between 0 and 1
	float SpecAmount = tileSpec.y;
	
	height+=terrainHeight;

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

#define CITY_LOD_LEVEL_HIGH 0
#define CITY_LOD_LEVEL_MED 1
#define CITY_LOD_LEVEL_LOW 2
#define CITY_LOD_LEVEL_XLOW 3
#define CITY_LOD_LEVEL_XXLOW 4

cbuffer CSCityPassCB : register( b0 )
{
	int2 tileCoords;
	uint tileLength;
	uint lodLevel;
}

AppendStructuredBuffer<Instance> sInstance : register(u0);

[numthreads(4, 4, 1)]
void CSCityPass(uint3 dispatchID : SV_DispatchThreadID)
{
	//return;
	float2 p1 = TILE_SIZE * dispatchID.xy - ((float)tileLength/2);
	float2 pos = tileCoords + p1;
	
	terrainData tData = loadTexMaps(pos);

	float noises[NOISE_ITERATIONS];
	float2 bounds[4];
	bool accept[4];

	getNoisesBoundsAccept(pos,tData,noises,bounds,accept);
	
	float terrainHeight;
	float tileCoeff;
	float4 tileCols;
	float2 tileSpec;

	getTerrainInfo(pos, tData, noises, accept, tileCoeff, terrainHeight, tileCols, tileSpec);
	
	if (terrainHeight > MAX_BUILDING_TERRAIN_HEIGHT) {
		return;
	}

	const float minBuildingHeight = 15;
	const float maxBuildingHeight = 450 * tileCoeff;
	
	float baseheight = (noise2D(pos.x/100,pos.y/72)/2)+0.5f;
	baseheight = minBuildingHeight + pow(baseheight,6)*(maxBuildingHeight);
	
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
	
	uint2 rn = poorRNG(pos)%3 + 1;
	
	const float maxFootPrint = (TILE_SIZE - (PAVE_WIDTH*2))/2;
	float2 fp = float2(maxFootPrint/rn.x,maxFootPrint/rn.y);
	
	const float2 bl = float2(PAVE_WIDTH,PAVE_WIDTH);
	
	float maxHeight = 0;

	for (uint i = 0; i < rn.x; i++) {
		for (uint j = 0; j < rn.y; j++) {
			if (i == 1 && j == 1 && rn.x == 3 && rn.y == 3) {
				continue;
			}
						
			float3 p;
			p.xz = p1 + bl + 2*fp * float2(i+0.5f,j+0.5f);
			
			float2 pos2 = tileCoords + p.xz;
			uint2 rnh = poorRNG(float2(pos2.x/120,pos2.y/98));
			float height = ( ((int)((rnh.x + rnh.y)%1024)) - 512)/512.0f;
			baseheight = baseheight + height * 0.15f * baseheight;
			
			p.y = terrainHeight-5 + baseheight;
			
			maxHeight = max(maxHeight,baseheight);
			
			[branch] if (lodLevel == CITY_LOD_LEVEL_HIGH) {
				Instance i0;
				i0.mPos = p;
				i0.mSize = float3(fp.x,baseheight,fp.y);
				i0.mColour = col;
				sInstance.Append(i0);
			}
		}
	}
	[branch] if (lodLevel != CITY_LOD_LEVEL_HIGH) {
	
		if (lodLevel >= CITY_LOD_LEVEL_LOW && maxHeight < maxBuildingHeight/8 + minBuildingHeight) {
			return;
		}
		if (lodLevel >= CITY_LOD_LEVEL_XLOW && maxHeight < maxBuildingHeight/4 + minBuildingHeight) {
			return;
		}
		if (lodLevel >= CITY_LOD_LEVEL_XXLOW && maxHeight < maxBuildingHeight/2 + minBuildingHeight) {
			return;
		}
	
		fp = float2(maxFootPrint,maxFootPrint);
		float3 p;
		p.xz = p1 + bl + 2*fp * float2(0.5f,0.5f);	
		p.y = terrainHeight-5 + maxHeight;
	
		Instance i0;
		i0.mPos = p;
		i0.mSize = float3(fp.x,maxHeight,fp.y);
		i0.mColour = col;
		sInstance.Append(i0);
	}
}

//*******UTILITY FUNCTIONS**********//

terrainData loadTexMaps(in float2 pos) {
	float scalex = 4000000;
	float scaley = scalex * (3190 / 2824);
	float2 scale = float2(-scalex,scaley);
	float2 scale2 = scale/2;
	float2 cpos = (pos + scale2)/scale;
	
	terrainData tData;
	tData.mBaseTex = baseTex.SampleLevel(sDefault,cpos,0);
	tData.mHeightTex = 2*heightTexMap.SampleLevel(sDefault,cpos,0);
	tData.mRoadMap = roadMap.SampleLevel(sDefault,cpos,0);
	tData.mCityMap = cityMap.SampleLevel(sDefault,cpos,0);
	
	return tData;
}

void getNoisesBoundsAccept( in float2 pos, in terrainData td, out float noises[NOISE_ITERATIONS], out float2 bounds[4], out bool accept[4]) {
	
	[loop] for (int i = 0; i < NOISE_ITERATIONS; i++) {
		float2 p2 = pos/(0.25f*scales[i]);
		[call] noises[i] = (noise2D(p2.x,p2.y)/2)+0.5f;
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
		
	float coeff;
	if (td.mRoadMap.b > 0.25f) {
		coeff = 0;
	}
	else {
		float3 city = td.mCityMap;
		float intensity = (city.r + city.g + city.b)/3.0f;
		coeff = intensity;
	}
	
	[unroll] for (int i = 0; i < 4; i++) {
		accept[i] = isAccepted(bounds[i], coeff);
	}
}

void getTerrainInfo(in float2 pos, in terrainData td, in float noises[NOISE_ITERATIONS], in bool accept[4], out float tileCoeff, out float terrainHeight, out float4 tileCols, out float2 tileSpec) {

	uint numAccept = 0;
	[unroll] for (uint i = 0; i < 4; i++) {
		numAccept+=accept[i] ? 1 : 0;
	}
	
	float coeff;
	if (td.mRoadMap.b > 0.25f) {
		coeff = 0;
	}
	else {
		float3 city = td.mCityMap;
		float intensity = (city.r + city.g + city.b)/3.0f;
		coeff = intensity;
	}
	tileCoeff = coeff;
	
/* 	tileCoeff = getTileCoeff(pos);
	uint tcr = round(tileCoeff);
	uint tc2 = tcr < tileCoeff ? tcr+1 : tcr-1;
	
	float4 colr = colours[tcr];
	float4 col2 = colours[tc2];
	
	//If there are roads and we're not in a road section, we need to change the coefficient to prevent interpolation
	if (numAccept && (colr.a != CITY_COEFF)) {
		tileCoeff = tc2;
	}
	//If the 2 coefficients are different, we need to prevent interpolation
	else if (colr.a != col2.a) {
		tileCoeff = tcr;
	}
	
	float ss = smoothstep(tcr,tc2,tileCoeff); */

	//Calculate the tile colour and speculars
	tileCols.rgb = td.mBaseTex;
	tileCols.a = 0;
	
	tileSpec = float2(128,0.1f);
	
	if (td.mRoadMap.b > 0.25f) {
		tileCols.rgb = lerp(tileCols.rgb,float3(0,0,0.5f),2*td.mRoadMap.b);
		tileSpec = (20,1);
	}
	
	//Calculate the terrain height
	terrainHeight = 1500 * td.mHeightTex.r;
	float heightFactor = max(0,1 - 2.0f*td.mRoadMap.b);
	terrainHeight*=heightFactor;
	
	 for (int i = 0; i < NOISE_ITERATIONS; i++) {
		//float c0 = coeffs[tcr][i];
		//float c1 = coeffs[tc2][i];
		//float mult = lerp(c0,c1,ss);
		float mult = 0.25f;
		terrainHeight+=noises[i] * mult;
	} 

}

float2 getShiftedCoords(float2 p) {
	return p;
	//float rn = (float)(poorRNG(p)%100)/100.0f;
	//return p + TILE_SIZE * pow(rn*0.50f,2);
}

bool isAccepted(float2 pos, float tileInd) {
	
	if (tileInd == 0) {
		return 0;
	}
	
	float2 pos1 = pos/1000;
	float2 pos2 = pos/500;
	float acc = 0;
	acc+=(poorRNG(pos).x%128)/256.0f;
	float accval = 0.2f * (1 - tileInd);
	bool accept = (acc>accval) ? 1 : 0;
	return accept;
}

float getTileCoeff(float2 pos) {
	const float scale = 30000;
	//n>0 always
	float n = (noise2D(pos.x/scale,pos.y/scale)/2)+0.501f;
	return min(NUM_BIOMES-1,NUM_BIOMES*n);
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