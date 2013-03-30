cbuffer VSConstantBuffer : register( b0 )
{
	matrix Model;
	matrix View;
	matrix Projection;
	matrix MV;
	matrix MVP;
}

cbuffer PSConstantBuffer : register( b1 )
{
	float3 CameraPos;
}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float3 ModelPos : POSITION0;
    float3 Norm : NORMAL0;
};

struct GBuffer
{
    float4 normal_specular : SV_Target0;
    float4 albedo : SV_Target1;
};

float2 EncodeSphereMap(float3 n)
{
    float oneMinusZ = 1.0f - n.z;
    float p = sqrt(n.x * n.x + n.y * n.y + oneMinusZ * oneMinusZ);
    return n.xy / p * 0.5f + 0.5f;
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

PS_INPUT VS( VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
	
	output.Pos = mul(input.Pos,MVP);
	output.ModelPos = mul(input.Pos,Model);
    output.Norm = normalize(mul(input.Norm,MV));
    return output;
}

// float4 PS( PS_INPUT input ) : SV_Target
// {
//	float3 normal = ((input.Norm.xyz)+1)/2;
	// return float4(input.Norm.xyz, 1.0f);
// }

GBuffer PS( PS_INPUT input )
{
	GBuffer output;
	float specularAmount = 1.0f;
	float specularPower = 1.0f;
	output.normal_specular = float4(EncodeSphereMap(input.Norm),specularAmount,specularPower);
	output.albedo = float4(1.0f,1.0f,1.0f,1.0f);
	return output;
	//float3 normal = ((input.Norm.xyz)+1)/2;
	//return float4(input.Norm.xyz, 1.0f);
}
