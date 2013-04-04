float4 UnpackRGBA16(uint2 e)
{
	return float4(f16tof32(e), f16tof32(e >> 16));
}

cbuffer PSCB : register( b0 )
{
	uint2 bufferDim;
}

Texture2D<float> depthTex : register(t0);
StructuredBuffer<uint2> litTex : register(t1);

struct LightingVSOutput {  
	float4 Pos : SV_POSITION;
	};

LightingVSOutput LightingVS(uint id : SV_VertexID) {
	LightingVSOutput output;
	float2 Tex = float2((id << 1) & 2, id & 2);
	output.Pos = float4(Tex * float2(2.0f, -2.0f) + 
		float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return output; }
	
float4 LightingPS(LightingVSOutput input) : SV_TARGET {

	int3 pix_pos = int3(input.Pos.xy,0);
	float depth = depthTex.Load(pix_pos);
	if (depth == 1.0f) {
	//Skybox
		return float4( 0.329f, 0.608f, 0.722f, 1.0f );
	}
	uint offset = (input.Pos.x - 0.5) + (input.Pos.y - 0.5) * bufferDim.x;
	float4 litSample = UnpackRGBA16(litTex[offset]);
	return litSample;
}