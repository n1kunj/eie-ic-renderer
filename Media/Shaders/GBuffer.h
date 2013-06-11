struct GBuffer
{
    float4 mNormSpec : SV_Target0;
    float4 mAlbedo : SV_Target1;
	float4 mEmittance : SV_Target2;
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

struct Instance {
	float3 mPos;
	float3 mSize;
	float3 mColour;
	float mRotY;
};