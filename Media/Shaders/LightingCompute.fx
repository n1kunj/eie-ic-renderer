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
	int3 coordOffset;
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
	
	
	//float3 lightVec = gbViewPos - lightLoc;

	float3 pixVal = float3(0,0,0);
	
	for (int i = 0; i < 10; i++) {
		
		PointLight pl = lightBuffer[i];
		
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
	//else {
	//	pixVal += ambient * gbAlbedo;
	//}
	//pixVal = float3(1.0f,0.0f,0.0f);
	
	if (all(screenPix.xy < bufferDim.xy)) {
		if (rawDepth != 1.0f) {
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