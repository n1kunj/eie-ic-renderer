#pragma once
#ifndef SHADERS_GBUFFERSHADER_H
#define SHADERS_GBUFFERSHADER_H
#include "..\DrawableShader.h"

class GBufferShader : public DrawableShader {
public:
	void DrawMesh(ID3D11DeviceContext* pd3dContext, const DrawableMesh* pMesh, const DrawableState* pState, const Camera* pCamera);
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice);
	void OnD3D11DestroyDevice();

	GBufferShader() : DrawableShader(L"GBufferShader") {}

	GBufferShader::~GBufferShader()
	{
		OnD3D11DestroyDevice();
	}

	//Statics
private:
	static boolean sCompiled;
	static ID3D11InputLayout* sVertexLayout;
	static ID3D11VertexShader* sVertexShader;
	static ID3D11PixelShader* sPixelShader;
	static ID3D11Buffer* sVSConstantBuffer;
	static ID3D11Buffer* sPSConstantBuffer;

};

#endif