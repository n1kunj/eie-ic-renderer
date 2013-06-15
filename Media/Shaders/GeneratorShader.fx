#include "SimplexNoise.fx"
#include "GBuffer.h"

#define DISTANT_TILE_GROUP_DIM 18
#define CITY_TILE_GROUP_DIM 8
#define TILE_SIZE 128

#define ROAD_WIDTH 12
#define BUILD_WIDTH 23
#define PAVE_WIDTH 35

#define OVERLAP_SCALE 1.05f

RWTexture2D<float4> albedoTex : register(u0);
RWTexture2D<float4> normalTex : register(u1);
RWTexture2D<float> heightTex : register(u2);

uint2 poorRNG(float2 xy);
float minDist(float2 l1, float2 l2, float2 p);

float getTileCoeff(float2 pos);
bool isAccepted(float2 pos, float tileInd);

cbuffer CSDistantTileCB : register( b0 )
{
	uint2 cDistantResolution;
	int2 cDistantTileCoords;
	uint cDistantTileSize;
}

#define NUM_BIOMES 9
#define NOISE_ITERATIONS 12

void getNoisesBoundsAccept( in float2 pos, out float noises[NOISE_ITERATIONS], out float2 bounds[4], out bool accept[4]);

void getTerrainInfo( in float2 pos, in float noises[NOISE_ITERATIONS], in bool accept[4], out float tileCoeff, out float terrainHeight, out float4 tileCols, out float2 tileSpec);

static const float scales[NOISE_ITERATIONS] = {
150000,	30000,	12800,	640,
320,	160,	80,		40,
20,		10,		5,		2.5
};

static const float coeffs[NUM_BIOMES][NOISE_ITERATIONS] = {
{0,0,2,1,0.5,0.5,0.5,0.5,0.5,0.5,0.25,0.25},

{0,0,0,0,0,0,0,0,0,0,0,0},//
{128,0,0,0,0,0,0,0,0,0,0,0},//
{256,0,0,0,0,0,0,0,0,0,0,0},//
{128,0,0,0,0,0,0,0,0,0,0,0},//
{0,0,0,0,0,0,0,0,0,0,0,0},//

// {0,0,0,0,0,0.25,0,0,0,0.25,0.25,0},
// {2048,512,128,64,32,16,8,4,2,1,0.5f,0.25f},
// {2048,512,128,64,32,16,8,4,2,1,0.5f,0.25f},
// {2048,512,128,64,32,16,8,4,2,1,0.5f,0.25f},
// {0,0,0,0,0,0.25,0,0,0,0.25,0.25,0},

{0,0,2,1,0.5,0.5,0.5,0.5,0.5,0.5,0.25,0.25},
{0,0,2,1,0.5,0.5,0.5,0.5,0.5,0.5,0.25,0.25},
{0,0,2,1,0.5,0.5,0.5,0.5,0.5,0.5,0.25,0.25},
};

#define CITY_COEFF 0

//r,g,b,city
static const float4 colours[NUM_BIOMES] = {
float4(0,0,0.5,1),

float4(0.9,0.9,0.9,0),
float4(0.9,0.9,0.9,0),
float4(0.9,0.9,0.9,0),
float4(0.9,0.9,0.9,0),
float4(0.9,0.9,0.9,0),

// float4(0.5,0.5,0.5,1),
// float4(0.5,0.5,0.5,1),
// float4(0.5,0.5,0.5,1),
// float4(0.5,0.5,0.5,1),
// float4(0.5,0.5,0.5,1),

float4(0,0,0.5,1),
float4(0,0,0.4,1),
float4(0,0,0.3,1),
};

static const float2 specPowAmount[NUM_BIOMES] = {
float2(20,1),
float2(128,0.1f),
float2(128,0.1f),
float2(128,0.1f),
float2(128,0.1f),
float2(128,0.1f),
float2(20,1),
float2(20,1),
float2(20,1),
};

groupshared float sDistantHeights[DISTANT_TILE_GROUP_DIM][DISTANT_TILE_GROUP_DIM];

[numthreads(DISTANT_TILE_GROUP_DIM, DISTANT_TILE_GROUP_DIM, 1)]
void CSDistantTile(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
	//Takes into account extra values around the edges
	int2 pixIndex = groupThreadID.xy - 1 + (DISTANT_TILE_GROUP_DIM-2) * groupID.xy;
	
	float2 pos = (float2)pixIndex/(cDistantResolution-1) - 0.50f;
	pos = pos * OVERLAP_SCALE * cDistantTileSize + cDistantTileCoords;
	
	float noises[NOISE_ITERATIONS];
	float2 bounds[4];
	bool accept[4];

	getNoisesBoundsAccept(pos,noises,bounds,accept);
	
	float terrainHeight;
	float tileCoeff;
	float4 tileCols;
	float2 tileSpec;

	getTerrainInfo(pos, noises, accept, tileCoeff, terrainHeight, tileCols, tileSpec);
	
	uint numAccept = 0;
	for (uint i = 0; i < 4; i++) {
		numAccept += accept[i] ? 1 : 0;
	}
	
	float3 colour = 0;
	float height = 0;
	
	if (tileCols.a == CITY_COEFF || numAccept) {
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
			height = 7.0f;
		}
		else if (roadDist < PAVE_WIDTH) {
			height = 7.15f;
			colour = float3(0.259,0.259,0.259);
		}
		else {
			colour = tileCols.rgb;
			height = 7.15f;
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
	sDistantHeights[groupThreadID.x][groupThreadID.y] = height;

	GroupMemoryBarrierWithGroupSync();
	
	float hp[4];
	hp[0] = sDistantHeights[groupThreadID.x-1][groupThreadID.y];
	hp[1] = sDistantHeights[groupThreadID.x+1][groupThreadID.y];
	hp[2] = sDistantHeights[groupThreadID.x][groupThreadID.y-1];
	hp[3] = sDistantHeights[groupThreadID.x][groupThreadID.y+1];
	
	float epsilon = (float)cDistantTileSize/(cDistantResolution.x-1);
	float3 va = normalize(float3(2*epsilon,hp[1]-hp[0],0));
	float3 vb = normalize(float3(0,hp[3]-hp[2],2*epsilon));	

	float3 normal = normalize(cross(vb,va));
	
	if (all(groupThreadID.xy > 0) && all(groupThreadID.xy < DISTANT_TILE_GROUP_DIM-1)) {
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

cbuffer CSCityTileCB : register( b0 )
{
	int2 cCityTileCoords;
	uint cCityTileSize;
	uint cCityLodLevel;
}

AppendStructuredBuffer<Instance> sInstance : register(u0);

[numthreads(CITY_TILE_GROUP_DIM, CITY_TILE_GROUP_DIM, 1)]
void CSCityTile(uint3 dispatchID : SV_DispatchThreadID)
{
	const float heightSeed = 300;
	const float minHeight = 30;
	float2 p1 = TILE_SIZE * dispatchID.xy - ((float)cCityTileSize/2);
	float2 pos = cCityTileCoords + p1;
	
	float baseheight = (noise2D(pos.x/100,pos.y/72)/2)+0.5f;
	baseheight = minHeight + pow(baseheight,4)*(heightSeed);
	
	float noises[NOISE_ITERATIONS];
	float2 bounds[4];
	bool accept[4];

	getNoisesBoundsAccept(pos,noises,bounds,accept);
	
	float terrainHeight;
	float tileCoeff;
	float4 tileCols;
	float2 tileSpec;

	getTerrainInfo(pos, noises, accept, tileCoeff, terrainHeight, tileCols, tileSpec);
	
	uint numAccept = 0;
	for (uint i = 0; i < 4; i++) {
		numAccept+=(accept[i] && accept[(i+1)%4]) ? 1 : 0;
	}
	
	if (numAccept <= 1) {
		return;
	}
	
	uint2 prng = poorRNG(pos);
		
	float3 col;
	col.r = (noise2D(pos.x,pos.y)/2)+0.5f;
	col.g = (noise2D(pos.x+900,pos.y-800)/2)+0.5f;
	col.b = pow((noise2D(pos.x-900,pos.y+8000)/2)+0.5f,2);
	
	float rotY = 0;
	float heightOffset = 0;
	float buildWidth = BUILD_WIDTH;
	if (cCityLodLevel == CITY_LOD_LEVEL_HIGH && baseheight > heightSeed/2) {
		heightOffset = 50;
		rotY = 0.10f * noise2D(pos.x/1000.0f,pos.y/1000.0f);
	}
	
	uint2 rn = prng%4 + 1;
	
	const float maxFootPrint = (TILE_SIZE - (BUILD_WIDTH*2))/2;
	float2 fp = float2(maxFootPrint/rn.x,maxFootPrint/rn.y);
	
	const float2 bl = float2(BUILD_WIDTH,BUILD_WIDTH);
	
	float maxHeight = 0;

	for (uint i = 0; i < rn.x; i++) {
		for (uint j = 0; j < rn.y; j++) {
			float3 p;
			p.xz = p1 + bl + 2*fp * float2(i+0.5f,j+0.5f);
			
			float2 pos2 = cCityTileCoords + p.xz;
			uint2 rnh = poorRNG(float2(pos2.x/120,pos2.y/98));
			float height = ( ((int)((rnh.x + rnh.y)%1024)) - 512)/512.0f;
			baseheight = baseheight + height * 0.15f * baseheight;
			
			p.y = terrainHeight-5 + baseheight + heightOffset;
			
			maxHeight = max(maxHeight,baseheight);
			
			[branch] if (cCityLodLevel == CITY_LOD_LEVEL_HIGH) {
				Instance i0;
				i0.mPos = p;
				i0.mSize = float3(fp.x,baseheight-heightOffset,fp.y);
				i0.mColour = col;
				i0.mRotY = rotY;
				sInstance.Append(i0);
			}
		}
	}
	[branch] if (cCityLodLevel != CITY_LOD_LEVEL_HIGH || heightOffset) {
	
		if (cCityLodLevel >= CITY_LOD_LEVEL_LOW && maxHeight < heightSeed/8 + minHeight) {
			return;
		}
		if (cCityLodLevel >= CITY_LOD_LEVEL_XLOW && maxHeight < heightSeed/4 + minHeight) {
			return;
		}
		if (cCityLodLevel >= CITY_LOD_LEVEL_XXLOW && maxHeight < heightSeed/2 + minHeight) {
			return;
		}
	
		fp = float2(maxFootPrint,maxFootPrint);
		
		float3 p;
		p.xz = p1 + bl + 2*fp * float2(0.5f,0.5f);
		
		if (heightOffset) {
			maxHeight = heightOffset;
			fp+=5;
		}
		
		p.y = terrainHeight-5 + maxHeight;

		Instance i0;
		i0.mPos = p;
		i0.mSize = float3(fp.x,maxHeight,fp.y);
		i0.mColour = col;
		i0.mRotY = 0;
		sInstance.Append(i0);
	}
}

//*******UTILITY FUNCTIONS**********//

void getNoisesBoundsAccept( in float2 pos, out float noises[NOISE_ITERATIONS], out float2 bounds[4], out bool accept[4]) {
	
	[loop] for (int i = 0; i < NOISE_ITERATIONS; i++) {
		float2 p2 = pos/(0.25f*scales[i]);
		[call] noises[i] = (noise2D(p2.x,p2.y)/2)+0.5f;
	}
	
	//Find the bottom left value of the quadrant we're assumed to be in
	int2 tile_id = floor(pos/TILE_SIZE);
	float2 bl = TILE_SIZE * tile_id;
	
	bounds[0] = bl;
	bounds[1] = bl + float2(TILE_SIZE,0);
	bounds[2] = bl + float2(TILE_SIZE,TILE_SIZE);
	bounds[3] = bl + float2(0,TILE_SIZE);
	
	[unroll] for (int i = 0; i < 4; i++) {
		accept[i] = isAccepted(bounds[i], getTileCoeff(bounds[i]));
	}
}

void getTerrainInfo( in float2 pos, in float noises[NOISE_ITERATIONS], in bool accept[4], out float tileCoeff, out float terrainHeight, out float4 tileCols, out float2 tileSpec) {

	uint numAccept = 0;
	[unroll] for (uint i = 0; i < 4; i++) {
		numAccept+=accept[i] ? 1 : 0;
	}
	
	tileCoeff = getTileCoeff(pos);
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
	
	float ss = smoothstep(tcr,tc2,tileCoeff);
	
	//Calculate the terrain height
	terrainHeight = 0;
	for (int i = 0; i < NOISE_ITERATIONS; i++) {
		float c0 = coeffs[tcr][i];
		float c1 = coeffs[tc2][i];
		float mult = lerp(c0,c1,ss);
		terrainHeight+=noises[i] * mult;
	}
	
	//Calculate the tile colour and speculars
	tileCols = lerp(colr,col2,ss);
	
	float2 specr = specPowAmount[tcr];
	float2 spec2 = specPowAmount[tc2];
	
	tileSpec = lerp(specr,spec2,ss);
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

bool isAccepted(float2 pos, float tileInd) {
	float4 cols = colours[(uint)round(tileInd)];
	if (cols.a != CITY_COEFF) {
		return 0;
	}

	float acc = 0;
	acc+=(poorRNG(pos).x%128)/256.0f - 0.4f;
	
	bool accept = (acc>-0.33f) ? 1 : 0;
	return accept;
}

float getTileCoeff(float2 pos) {
	const float scale = 30000;
	//n>0 always
	float n = (noise2D(pos.x/scale,pos.y/scale)/2)+0.501f;
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