#define CS_GROUP_DIM 16

RWTexture2D<float4> heightTex : register(t0);

[numthreads(CS_GROUP_DIM, CS_GROUP_DIM, 1)]
void HeightMapCS(uint3 groupId 			: SV_GroupID,
				uint3 dispatchThreadId 	: SV_DispatchThreadID,
				uint3 groupThreadId    	: SV_GroupThreadID,
				uint groupIndex 		: SV_GroupIndex)
{
	heightTex[dispatchThreadId.xy] = float4(0.0f,1.0f,0.0f,0.0f);
}