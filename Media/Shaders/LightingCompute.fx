#define MAX_LIGHTS 1024

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
	matrix Projection;
	matrix View;
	uint2 bufferDim;
	int3 coordOffset;
	float3 lightLoc;
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

float3 ComputePositionViewFromZ(float2 positionScreen,
                                float viewSpaceZ)
{
    float2 screenSpaceRay = float2(positionScreen.x / Projection._11,
                                   positionScreen.y / Projection._22);
    
    float3 positionView;
    positionView.z = viewSpaceZ;
    // Solve the two projection equations
    positionView.xy = screenSpaceRay.xy * positionView.z;
    
    return positionView;
}

void WriteSample(uint2 coords, float4 value)
{
	uint addr = coords.y * bufferDim.x + coords.x;
	gFramebuffer[addr] = PackRGBA16(value);
}

[numthreads(COMPUTE_SHADER_TILE_GROUP_DIM, COMPUTE_SHADER_TILE_GROUP_DIM, 1)]
void LightingCS(uint3 groupId 			: SV_GroupID,
				uint3 dispatchThreadId 	: SV_DispatchThreadID,
				uint3 groupThreadId    	: SV_GroupThreadID,
				uint groupIndex 		: SV_GroupIndex )
{

	uint2 globalCoords = dispatchThreadId.xy;
	int3 pix_pos = int3(globalCoords,0);
	float depth = depthTex.Load(pix_pos);
	float4 nor_spec = normals_specular.Load(pix_pos);
	float3 alb = albedo.Load(pix_pos).xyz;
	float specAmount = nor_spec.z;
	float specExp = nor_spec.w;
	float3 normal = DecodeSphereMap(nor_spec.xy);
	float3 positionView;
	{
		float2 screenPixelOffset = float2(2.0f, -2.0f) / bufferDim;
		float2 positionScreen = (float2(globalCoords.xy) + 0.5f) * screenPixelOffset.xy + float2(-1.0f, 1.0f);
		
		float viewSpaceZ = Projection[3][2] / ( depth - Projection[2][2]);
		
		positionView = ComputePositionViewFromZ(positionScreen, viewSpaceZ);
	}
	
	float3 ll = lightLoc;
	
	float3 lightDir = normalize(positionView.xyz - ll);
	float diffuse = saturate(dot(normal, -lightDir));
	
	float3 dalb = diffuse * alb;
	
	if (all(globalCoords < bufferDim.xy)) {
		if (depth != 1.0f) {
			WriteSample(globalCoords, float4(dalb, 0.0f));
		}
	}
}