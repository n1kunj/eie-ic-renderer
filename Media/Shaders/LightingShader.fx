float3 calculateViewPos(uint2 pScreenPix, uint2 pBufferDim, float pRawDepth);

float4 UnpackRGBA16(uint2 e)
{
	return float4(f16tof32(e), f16tof32(e >> 16));
}

cbuffer PSCB : register( b0 )
{
	matrix Projection;
	uint2 bufferDim;
	float yHeight;
}

Texture2D<float> depthTex : register(t0);
StructuredBuffer<uint2> litTex : register(t1);


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
	
	float3 viewPos = calculateViewPos(pix_pos.xy,bufferDim,depth);
	
	float3 fogCol = float3( 0.501f, 0.855f, 0.921f);
	
	if (depth == 1.0f) {
	//Skybox
		return float4(fogCol, 1.0f );
	}
	
	uint offset = (input.Pos.x - 0.5) + (input.Pos.y - 0.5) * bufferDim.x;
	float4 litSample = UnpackRGBA16(litTex[offset]);
	
	float fogEnd = 3200000;
	float fogStart = 3200000/2;
	
	fogEnd = sqrt(fogEnd*fogEnd + yHeight*yHeight);
	fogStart = sqrt(fogStart*fogStart + yHeight*yHeight);
	
	float fogFactor = saturate((fogEnd - length(viewPos))/(fogEnd - fogStart));
	fogFactor = pow(fogFactor,1.0/3.0);

	litSample.xyz = lerp(fogCol,litSample,fogFactor);
	return litSample;
}

float3 calculateViewPos(uint2 pScreenPix, uint2 pBufferDim, float pRawDepth) {
	float2 pixOffset = float2(2.0f, -2.0f) / pBufferDim;
	float2 screenPos = (float2(pScreenPix) + 0.5f) * pixOffset + float2(-1.0f, 1.0f);
	float realDepth = Projection[3][2] / ( pRawDepth - Projection[2][2]);
	float2 viewRay = float2(screenPos/float2(Projection[0][0],Projection[1][1]));
	
	return float3(viewRay*realDepth, realDepth);
}