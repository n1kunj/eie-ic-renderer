#include "SimplexNoise.fx"
#include "GBuffer.h"

#define DISTANT_TILE_GROUP_DIM 18
#define CITY_TILE_GROUP_DIM 8
#define CITY_GRID_SIZE 128

#define ROAD_WIDTH 12
#define BUILD_WIDTH 23
#define PAVE_WIDTH 35

#define OVERLAP_SCALE 1.05f

RWTexture2D<float4> albedoTex : register(u0);
RWTexture2D<float4> normalTex : register(u1);
RWTexture2D<float> heightTex : register(u2);

StructuredBuffer<float> scalesBuffer : register(t1);
StructuredBuffer<float> coeffsBuffer : register(t2);
StructuredBuffer<float4> coloursBuffer : register(t3);
StructuredBuffer<float2> specPowBuffer : register(t4);

float getTileCoeff(float2 pos);
bool isAccepted(float2 pos, float tileInd);
float minDist(float2 l1, float2 l2, float2 p);
uint2 poorRNG(float2 xy);

cbuffer CSGlobalTileCB : register(b0)
{
	uint cNoiseIterations;
	uint cNumBiomes;
}

cbuffer CSDistantTileCB : register(b1)
{
	uint2 cDistantResolution;
	int2 cDistantTileCoords;
	uint cDistantTileSize;
}

#define MAX_NOISE_ITERATIONS 16

void getNoisesBoundsAccept( in float2 pos, out float noises[MAX_NOISE_ITERATIONS], out float2 bounds[4], out bool accept[4]);

void getTerrainInfo( in float2 pos, in float noises[MAX_NOISE_ITERATIONS], in bool accept[4], out float tileCoeff, out float terrainHeight, out float4 tileCols, out float2 tileSpec);

#define MAX_CITY_COEFF 0

groupshared float sDistantHeights[DISTANT_TILE_GROUP_DIM][DISTANT_TILE_GROUP_DIM];

[numthreads(DISTANT_TILE_GROUP_DIM, DISTANT_TILE_GROUP_DIM, 1)]
void CSDistantTile(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
	//Takes into account extra values around the edges
	int2 pixIndex = groupThreadID.xy - 1 + (DISTANT_TILE_GROUP_DIM-2) * groupID.xy;
	
	float2 pos = (float2)pixIndex/(cDistantResolution-1) - 0.50f;
	pos = pos * OVERLAP_SCALE * cDistantTileSize + cDistantTileCoords;
	
	float noises[MAX_NOISE_ITERATIONS];
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
	
	if (tileCols.a <= MAX_CITY_COEFF || numAccept) {
		float roadDist = 9999999.0f;

		for (int i = 0; i < 4; i++) {
			int ind1 = i%4;
			int ind2 = (i+1)%4;
			
			if (accept[ind1]) {
				float dist;
				if (accept[ind2]) {
					dist = minDist(bounds[ind1],bounds[ind2],pos);
				}
				else {
					dist = length(bounds[ind1] - pos);
				}
				roadDist = min(roadDist,dist);
			}
		}
		
		if (roadDist < ROAD_WIDTH) {
			colour = float3(0.55,0.52,0.52);
		}
		else if (roadDist < PAVE_WIDTH) {
			colour = float3(0.259,0.259,0.259);
		}
		else {
			colour = tileCols.rgb;
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

cbuffer CSCityTileCB : register(b1)
{
	int2 cCityTileCoords;
	uint cCityTileSize;
	uint cCityLodLevel;
}

AppendStructuredBuffer<Instance> sInstance : register(u0);

[numthreads(CITY_TILE_GROUP_DIM, CITY_TILE_GROUP_DIM, 1)]
void CSCityTile(uint3 dispatchID : SV_DispatchThreadID)
{
	float2 p1 = CITY_GRID_SIZE * dispatchID.xy - ((float)cCityTileSize/2);
	float2 pos = cCityTileCoords + p1;
	
	float noises[MAX_NOISE_ITERATIONS];
	float2 bounds[4];
	bool accept[4];

	getNoisesBoundsAccept(pos,noises,bounds,accept);
	
	float terrainHeight;
	float tileCoeff;
	float4 tileCols;
	float2 tileSpec;

	getTerrainInfo(pos, noises, accept, tileCoeff, terrainHeight, tileCols, tileSpec);
	
	const float heightSeed = -tileCols.a;
	const float minHeight = 20;
	
	float baseheight = (noise2D(pos.x/100,pos.y/72)/2)+0.5f;
	baseheight = minHeight + pow(baseheight,4)*(heightSeed);
	
	uint numAccept = 0;
	for (uint i = 0; i < 4; i++) {
		numAccept+=(accept[i] && accept[(i+1)%4]) ? 1 : 0;
	}
	
	if (numAccept <= 1) {
		return;
	}
	
	uint2 prng = poorRNG(pos);
		
	float3 col;
	col.r = prng.x%256/256.0;
	col.g = prng.y%256/256.0;
	col.b = pow((prng.y+prng.x)%1024/1024.0,2);
	
	float rotY = 0;
	float heightOffset = 0;
	float buildWidth = BUILD_WIDTH;
	if (cCityLodLevel == CITY_LOD_LEVEL_HIGH && baseheight > 100) {
		heightOffset = 50;
		rotY = 0.10f * noise2D(pos.x/1000.0f,pos.y/1000.0f);
	}
	
	uint2 rn = prng%4 + 1;
	
	const float maxFootPrint = (CITY_GRID_SIZE - (BUILD_WIDTH*2))/2;
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
			
			p.y = terrainHeight-10 + baseheight + heightOffset;
			
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
		
		p.y = terrainHeight-10 + maxHeight;

		Instance i0;
		i0.mPos = p;
		i0.mSize = float3(fp.x,maxHeight,fp.y);
		i0.mColour = col;
		i0.mRotY = 0;
		sInstance.Append(i0);
	}
}

//*******UTILITY FUNCTIONS**********//

void getNoisesBoundsAccept( in float2 pos, out float noises[MAX_NOISE_ITERATIONS], out float2 bounds[4], out bool accept[4]) {
	
	[loop] for (int i = 0; i < MAX_NOISE_ITERATIONS; i++) {
		float2 p2 = pos/(0.25f*scalesBuffer[i]);
		[call] noises[i] = (noise2D(p2.x,p2.y)/2)+0.5f;
	}
	
	//Find the bottom left value of the quadrant we're assumed to be in
	int2 tile_id = floor(pos/CITY_GRID_SIZE);
	float2 bl = CITY_GRID_SIZE * tile_id;
	
	bounds[0] = bl;
	bounds[1] = bl + float2(CITY_GRID_SIZE,0);
	bounds[2] = bl + float2(CITY_GRID_SIZE,CITY_GRID_SIZE);
	bounds[3] = bl + float2(0,CITY_GRID_SIZE);
	
	[unroll] for (int i = 0; i < 4; i++) {
		accept[i] = isAccepted(bounds[i], getTileCoeff(bounds[i]));
	}
}

void getTerrainInfo( in float2 pos, in float noises[MAX_NOISE_ITERATIONS], in bool accept[4], out float tileCoeff, out float terrainHeight, out float4 tileCols, out float2 tileSpec) {

	uint numAccept = 0;
	[unroll] for (uint i = 0; i < 4; i++) {
		numAccept+=accept[i] ? 1 : 0;
	}
	
	tileCoeff = getTileCoeff(pos);
	uint tcr = round(tileCoeff);
	uint tc2 = tcr < tileCoeff ? tcr+1 : tcr-1;
	
	float4 colr = coloursBuffer[tcr];
	float4 col2 = coloursBuffer[tc2];
	
	//If there are roads and we're not in a road section, we need to change the coefficient to prevent interpolation
	if (numAccept && (colr.a > MAX_CITY_COEFF)) {
		tileCoeff = tc2;
	}
	//If the 2 coefficients are different, we need to prevent interpolation
	else if (colr.a != col2.a) {
		tileCoeff = tcr;
	}
	
	float ss = smoothstep(tcr,tc2,tileCoeff);
	
	//Calculate the terrain height
	terrainHeight = 0;
	for (int i = 0; i < MAX_NOISE_ITERATIONS; i++) {
		float c0 = coeffsBuffer[tcr * cNoiseIterations + i];
		float c1 = coeffsBuffer[tc2 * cNoiseIterations + i];
		float mult = lerp(c0,c1,ss);
		float height = ((noises[i]/2)+0.5f) * mult;
		terrainHeight += i < cNoiseIterations ? height : 0;
	}
	
	//Calculate the tile colour and speculars
	tileCols = lerp(colr,col2,ss);
	
	float2 specr = specPowBuffer[tcr];
	float2 spec2 = specPowBuffer[tc2];
	
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
	float4 cols = coloursBuffer[(uint)round(tileInd)];
	if (cols.a > MAX_CITY_COEFF) {
		return 0;
	}

	float acc = 0;
	acc+=(poorRNG(pos).x%128)/256.0f - 0.4f;
	
	bool accept = (acc>-0.33f) ? 1 : 0;
	return accept;
}

float getTileCoeff(float2 pos) {
	float n = 0;
	const uint numNoises = 3;
	for (uint i = 0; i < numNoises; i++) {
		float2 pos2 = pos/scalesBuffer[i];
		float noise = noise2D(pos2.x,pos2.y);
		n += pow(2,numNoises-1-i)*noise;
	}
	
	n = n/(2*(pow(2,numNoises)-1)) + 0.501f;
	
	return min(cNumBiomes-1,cNumBiomes*n);
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