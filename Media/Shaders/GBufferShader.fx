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

PS_INPUT VS( VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
	
	output.Pos = mul(input.Pos,MVP);
	output.ModelPos = mul(input.Pos,Model);
    output.Norm = normalize(mul(input.Norm,Model));
    return output;
}

float4 PS( PS_INPUT input ) : SV_Target
{
	//float3 normal = ((input.Norm.xyz)+1)/2;
	return float4(input.Norm.xyz, 1.0f);
}
