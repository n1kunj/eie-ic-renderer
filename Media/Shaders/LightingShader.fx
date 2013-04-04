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

float4 UnpackRGBA16(uint2 e)
{
    return float4(f16tof32(e), f16tof32(e >> 16));
}

cbuffer PSCB : register( b0 )
{
	uint2 bufferDim;
}

Texture2D normals_specular : register(t0);
Texture2D albedo : register(t1);
Texture2D depthTex : register(t2);
StructuredBuffer<uint2> litTex : register(t3);

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

	uint offset = (input.Pos.x - 0.5) + (input.Pos.y - 0.5) * bufferDim.x;
	float4 litSample = UnpackRGBA16(litTex[offset]);
	return litSample;
}