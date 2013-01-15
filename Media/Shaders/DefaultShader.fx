cbuffer ConstantBuffer : register( b0 )
{
	matrix Model;
	matrix View;
	matrix Projection;
	matrix MVP;
}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : TEXCOORD0;
};

PS_INPUT VS( VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
	
	output.Pos = mul(input.Pos,MVP);
	
    output.Norm = mul(input.Norm,View);
    return output;
}

float4 PS( PS_INPUT input ) : SV_Target
{
	float3 norm = (input.Norm / 2.0f) + 0.5f;
	return float4(norm,1.0f);
}
