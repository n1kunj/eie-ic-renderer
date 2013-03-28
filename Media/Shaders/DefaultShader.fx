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
	float3 DiffuseColour;
	float BlinnPhongExponent;
	float3 AmbientColour;
	float3 SpecularColour;
	float3 DiffuseLightColour;
	float3 AmbientLightColour;
	float3 LightPos;
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
	output.ModelPos = mul(input.Pos,MV);
    output.Norm = normalize(mul(input.Norm,Model));
    return output;
}

float4 PS( PS_INPUT input ) : SV_Target
{
	float3 lightVec = normalize(LightPos - input.ModelPos);

	float diffuse = saturate( dot(lightVec, input.Norm));

	float3 cameraVec = normalize(CameraPos - input.ModelPos);

	float3 halfway = normalize(cameraVec + lightVec);

	float specular = pow(saturate(dot(halfway, input.Norm)),BlinnPhongExponent);

	float3 diffuseOut = diffuse * DiffuseColour * DiffuseLightColour;

	float3 specularOut = specular * SpecularColour;

	float3 ambientOut = AmbientLightColour * AmbientColour;

	float3 colour = ambientOut + diffuseOut + specularOut;

	return float4( colour, 1.0f);
}
