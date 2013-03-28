#pragma once
#ifndef SHADERS_DEFERREDTILECOMPUTESHADER_H
#define SHADERS_DEFERREDTILECOMPUTESHADER_H
#include "..\PostShader.h"

class DeferredTileComputeShader : public PostShader {
public:
	void DrawPost(ID3D11DeviceContext* pd3dDeviceContext);
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice);
	void OnD3D11DestroyDevice();
	DeferredTileComputeShader();
	~DeferredTileComputeShader();
private:

};

#endif