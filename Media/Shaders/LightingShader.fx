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

Texture2D normals_specular : register(t0);
Texture2D albedo : register(t1);
Texture2D depthTex : register(t2);

SamplerState samLinear
{
    Filter = NEAREST;
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
	float depth = depthTex.Sample(samLinear,input.Tex);
	//Early return
	if (depth == 1.0f) {
		//discard;
		return 1.0f;
		//When we implement a skybox post process we can just discard and get a slight perf boost
	}
	float4 nor_spec = normals_specular.Sample(samLinear,input.Tex);
	float4 alb = albedo.Sample(samLinear,input.Tex);
	float3 normal = DecodeSphereMap(nor_spec.xy);
	return float4(normal,1.0f);
}