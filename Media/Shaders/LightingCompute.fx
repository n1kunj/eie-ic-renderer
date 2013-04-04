#define MAX_LIGHTS 1024

// This determines the tile size for light binning and associated tradeoffs
#define COMPUTE_SHADER_TILE_GROUP_DIM 16
#define COMPUTE_SHADER_TILE_GROUP_SIZE (COMPUTE_SHADER_TILE_GROUP_DIM*COMPUTE_SHADER_TILE_GROUP_DIM)

uint2 PackRGBA16(float4 c)
{
	return f32tof16(c.rg) | (f32tof16(c.ba) << 16);
}

groupshared uint sMinZ;
groupshared uint sMaxZ;

// Light list for the tile
groupshared uint sTileLightIndices[MAX_LIGHTS];
groupshared uint sTileNumLights;

Texture2D<float4> normals_specular : register(t0);
Texture2D<float4> albedo : register(t1);
Texture2D<float> depthTex : register(t2);
RWStructuredBuffer<uint2> gFramebuffer : register(u0);

cbuffer LightingCSCB : register( b0 )
{
	uint2 bufferDim;
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

void WriteSample(uint2 coords, float4 value)
{
	uint addr = coords.y * bufferDim.x + coords.x;
	gFramebuffer[addr] = PackRGBA16(value);
}

[numthreads(COMPUTE_SHADER_TILE_GROUP_DIM, COMPUTE_SHADER_TILE_GROUP_DIM, 1)]
void LightingCS(uint3 groupId          : SV_GroupID,
						 uint3 dispatchThreadId : SV_DispatchThreadID,
						 uint3 groupThreadId    : SV_GroupThreadID,
						 uint groupIndex : SV_GroupIndex
						 )
{
	uint2 globalCoords = dispatchThreadId.xy;
	int3 pix_pos = int3(globalCoords,0);
	float depth = depthTex.Load(pix_pos);
	float4 nor_spec = normals_specular.Load(pix_pos);
	float4 alb = albedo.Load(pix_pos);
	float3 normal = DecodeSphereMap(nor_spec.xy);
	normal/=2;
	normal+=0.5f;
	
	if (depth == 1.0f) {
		WriteSample(globalCoords, float4(0.0f, 0.0f, 0.0f, 0.0f));
	}
	else if (all(globalCoords < bufferDim.xy)) {
		WriteSample(globalCoords, float4(normal, 0.0f));
	}
}