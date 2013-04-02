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

cbuffer PSCB : register( b0 )
{
	matrix Projection;
}

Texture2D normals_specular : register(t0);
Texture2D albedo : register(t1);
Texture2D depthTex : register(t2);

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
	float4 nor_spec = normals_specular.Load(pix_pos);
	float4 alb = albedo.Load(pix_pos);
	float3 normal = DecodeSphereMap(nor_spec.xy);
	normal/2;
	normal+=0.5f;
	return float4(normal,1.0f);
}