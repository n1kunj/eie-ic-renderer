Texture2D	 inputTexture       : register(t0);

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct LightingVSOutput {  
	float4 Pos : SV_POSITION;              
	float2 Tex : TEXCOORD0; };

LightingVSOutput LightingVS(uint id : SV_VertexID) {
	LightingVSOutput output;
	output.Tex = float2((id << 1) & 2, id & 2);
	output.Pos = float4(output.Tex * float2(2.0f, -2.0f) + 
		float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return output; }
	
float4 LightingPS(LightingVSOutput input) : SV_TARGET {
//discard;
	return inputTexture.Sample(samLinear,input.Tex);
}