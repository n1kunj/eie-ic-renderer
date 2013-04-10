#define MAX_LIGHTS 1024

#define COMPUTE_SHADER_TILE_GROUP_DIM 16
#define COMPUTE_SHADER_TILE_GROUP_SIZE (COMPUTE_SHADER_TILE_GROUP_DIM*COMPUTE_SHADER_TILE_GROUP_DIM)

struct PointLight {
	float3 viewPos;
	float attenEnd;
	float3 colour;
	float ambient;
};

uint2 PackRGBA16(float4 c)
{
	return f32tof16(c.rg) | (f32tof16(c.ba) << 16);
}

float3 DecodeSphereMap(float2 e)
{
	float2 tmp = e - e * e;
	float f = tmp.x + tmp.y;
	float m = sqrt(4.0f * f - 1.0f);
	
	float3 n;
	n.xy = m * (e * 4.0f - 2.0f);
	n.z  = 3.0f - 8.0f * f;
	return n;
}

float3 calculateViewPos(uint2 pScreenPix, uint2 pBufferDim, float pRawDepth);
void WriteSample(uint2 coords, uint2 gBufferDim, float4 value);

cbuffer LightingCSCB : register( b0 )
{
	matrix Projection;
	uint2 bufferDim;
	float2 zNearFar;
	int3 coordOffset;
	uint numLights;
	float3 lightLoc;
}

groupshared uint sMinZ;
groupshared uint sMaxZ;

// Light list for the tile
groupshared uint sTileLightIndices[MAX_LIGHTS];
groupshared uint sTileNumLights;

Texture2D<float4> norSpecTex : register(t0);
Texture2D<float4> albedoTex : register(t1);
Texture2D<float> depthTex : register(t2);
StructuredBuffer<PointLight> lightBuffer : register(t3);
RWStructuredBuffer<uint2> gFrameBuffer : register(u0);


[numthreads(COMPUTE_SHADER_TILE_GROUP_DIM, COMPUTE_SHADER_TILE_GROUP_DIM, 1)]
void LightingCS(uint3 groupId 			: SV_GroupID,
				uint3 dispatchThreadId 	: SV_DispatchThreadID,
				uint3 groupThreadId    	: SV_GroupThreadID,
				uint groupIndex 		: SV_GroupIndex )
{
	uint3 screenPix = uint3(dispatchThreadId.xy,0);
	
	float rawDepth = depthTex.Load(screenPix);
	
	float4 norSpec = norSpecTex.Load(screenPix);
	
	float3 gbAlbedo = albedoTex.Load(screenPix).xyz;
	float3 gbNormal = DecodeSphereMap(norSpec.xy);
	float gbSpecAmount = norSpec.z;
	float gbSpecExp = norSpec.w;
	float3 gbViewPos = calculateViewPos(screenPix.xy, bufferDim, rawDepth);
	
	//Outside of screen pixels break tiling
	if (!all(screenPix.xy < bufferDim.xy)) {
		gbViewPos.z = 1.0f;
	}
	
	float minZSample = zNearFar.y; //large
	float maxZSample = zNearFar.x; //small
	{
		float viewSpaceZ = gbViewPos.z;
		bool validPixel = viewSpaceZ >= maxZSample && viewSpaceZ < minZSample;
		[flatten] if (validPixel) {
			minZSample = min(minZSample, viewSpaceZ);
			maxZSample = max(maxZSample, viewSpaceZ);
		}
	}
	
	// Initialize shared memory light list and Z bounds
	if (groupIndex == 0) {
		sTileNumLights = 0;
		sMinZ = 0x7F7FFFFF;      // Max float
		sMaxZ = 0;
	}

	GroupMemoryBarrierWithGroupSync();
	
	if (maxZSample >= minZSample) {
		InterlockedMin(sMinZ, asuint(minZSample));
		InterlockedMax(sMaxZ, asuint(maxZSample));
	}

	GroupMemoryBarrierWithGroupSync();

	float minTileZ = asfloat(sMinZ);
	float maxTileZ = asfloat(sMaxZ);
	
	float2 tileScale = float2(bufferDim.xy) * rcp(float(2 * COMPUTE_SHADER_TILE_GROUP_DIM));
	float2 tileBias = tileScale - float2(groupId.xy);
	
	// Now work out composite projection matrix
	// Relevant matrix columns for this tile frusta
	float4 c1 = float4(Projection._11 * tileScale.x, 0.0f, tileBias.x, 0.0f);
	float4 c2 = float4(0.0f, -Projection._22 * tileScale.y, tileBias.y, 0.0f);
	float4 c4 = float4(0.0f, 0.0f, 1.0f, 0.0f);

	// Derive frustum planes
	float4 frustumPlanes[6];
	// Sides
	frustumPlanes[0] = c4 - c1;
	frustumPlanes[1] = c4 + c1;
	frustumPlanes[2] = c4 - c2;
	frustumPlanes[3] = c4 + c2;
	// Near/far
	frustumPlanes[4] = float4(0.0f, 0.0f,  1.0f, -minTileZ);
	frustumPlanes[5] = float4(0.0f, 0.0f, -1.0f,  maxTileZ);
	
	// Normalize frustum planes (near/far already normalized)
	[unroll] for (uint i = 0; i < 4; ++i) {
		frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
	}
	
	// Cull lights for this tile
	for (uint lightIndex = groupIndex; lightIndex < numLights; lightIndex += COMPUTE_SHADER_TILE_GROUP_SIZE) {
		PointLight light = lightBuffer[lightIndex];
				
		// Cull: point light sphere vs tile frustum
		bool inFrustum = true;
		[unroll] for (uint i = 0; i < 6; ++i) {
			float d = dot(frustumPlanes[i], float4(light.viewPos, 1.0f));
			inFrustum = inFrustum && (d >= -light.attenEnd);
		}

		[branch] if (inFrustum) {
			// Append light to list
			// Compaction might be better if we expect a lot of lights
			uint listIndex;
			InterlockedAdd(sTileNumLights, 1, listIndex);
			sTileLightIndices[listIndex] = lightIndex;
		}
	}
	
	GroupMemoryBarrierWithGroupSync();
	
	uint lightCount = sTileNumLights;
	
	float3 pixVal = float3(0,0,0);
	
	for (int i = 0; i < lightCount; i++) {
		PointLight pl = lightBuffer[sTileLightIndices[i]];
		
		float3 lightVec = gbViewPos - pl.viewPos;

		float lightDist = length(lightVec);
		lightVec = normalize(lightVec);
		float diffuse = dot(gbNormal, -lightVec);
		
		float ambient = pl.ambient;
		
		float attenEnd = pl.attenEnd;
		
		float lightFactor = 0.0f;
		if (attenEnd > lightDist) {
			lightFactor = 1 - sqrt(lightDist/attenEnd);
		}

		if (diffuse > 0 && lightFactor > 0.0f) {
			float3 cameraVec = normalize(-gbViewPos);

			float3 reflected = reflect(lightVec, gbNormal);
			float rdotv = max(0.0f, dot(reflected,cameraVec));
			float specular = pow(rdotv, gbSpecExp);
			pixVal += ((ambient + diffuse + specular*gbSpecAmount) * lightFactor) * gbAlbedo * pl.colour;
		}
	}
	
	if (all(screenPix.xy < bufferDim.xy)) {
		if (rawDepth != 1.0f) {
			//WriteSample(screenPix.xy,bufferDim.xy, float4(lightCount.xxx/512.0f, 0.0f));
			WriteSample(screenPix.xy,bufferDim.xy, float4(pixVal, 0.0f));
		}
	}
}

float3 calculateViewPos(uint2 pScreenPix, uint2 pBufferDim, float pRawDepth) {
	float2 pixOffset = float2(2.0f, -2.0f) / pBufferDim;
	float2 screenPos = (float2(pScreenPix) + 0.5f) * pixOffset + float2(-1.0f, 1.0f);
	float realDepth = Projection[3][2] / ( pRawDepth - Projection[2][2]);
	float2 viewRay = float2(screenPos/float2(Projection[0][0],Projection[1][1]));
	
	return float3(viewRay*realDepth, realDepth);
}

void WriteSample(uint2 coords, uint2 gBufferDim, float4 value)
{
	uint addr = coords.y * gBufferDim.x + coords.x;
	gFrameBuffer[addr] = PackRGBA16(value);
}