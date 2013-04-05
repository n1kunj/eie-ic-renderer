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
	float3 Albedo;
	float SpecPower;
	float SpecAmount;
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
	float3 lil : POSITION1;
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

PS_INPUT VS( VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
	
	output.Pos = mul(input.Pos,MVP);
	output.ModelPos = mul(input.Pos,MV);
    output.Norm = normalize(mul(input.Norm,MV));
	float4 ll = mul(float4(5,3,2,1),View);
	output.lil = ll.xyz/ll.w;
    return output;
}

GBuffer PS( PS_INPUT input )
{
	//float3 ll = float3(5,3,2);
	//ll = input.lil;
	
	GBuffer output;
	output.normal_specular = float4(EncodeSphereMap(input.Norm),SpecAmount,SpecPower);
	output.albedo = float4(Albedo,1.0f);
	
	return output;
}
