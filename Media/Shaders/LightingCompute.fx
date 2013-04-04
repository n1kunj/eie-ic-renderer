// Copyright 2010 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

#define MAX_LIGHTS 1024

// This determines the tile size for light binning and associated tradeoffs
#define COMPUTE_SHADER_TILE_GROUP_DIM 16
#define COMPUTE_SHADER_TILE_GROUP_SIZE (COMPUTE_SHADER_TILE_GROUP_DIM*COMPUTE_SHADER_TILE_GROUP_DIM)

groupshared uint sMinZ;
groupshared uint sMaxZ;

// Light list for the tile
groupshared uint sTileLightIndices[MAX_LIGHTS];
groupshared uint sTileNumLights;

Texture2D normals_specular : register(t0);
Texture2D albedo : register(t1);
Texture2D depthTex : register(t2);
RWStructuredBuffer<uint2> gFramebuffer : register(u0);

cbuffer LightingCSCB : register( b0 )
{
	uint2 bufferDim;
}

uint2 PackRGBA16(float4 c)
{
	return f32tof16(c.rg) | (f32tof16(c.ba) << 16);
}

void WriteSample(uint2 coords, float4 value)
{
	uint addr = coords.y * bufferDim.x + coords.x;
	gFramebuffer[addr] = PackRGBA16(value);
}

[numthreads(COMPUTE_SHADER_TILE_GROUP_DIM, COMPUTE_SHADER_TILE_GROUP_DIM, 1)]
void LightingCS(uint3 groupId          : SV_GroupID,
						 uint3 dispatchThreadId : SV_DispatchThreadID,
						 uint3 groupThreadId    : SV_GroupThreadID,
						 uint groupIndex : SV_GroupIndex
						 )
{
	uint2 globalCoords = dispatchThreadId.xy;
	if (all(globalCoords < bufferDim.xy)) {
		WriteSample(globalCoords, float4(1.0f, 0.0f, 0.0f, 0.0f));
	}
}