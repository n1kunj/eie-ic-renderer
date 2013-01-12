#ifndef SHADERS_DEFAULTSHADER_H
#define SHADERS_DEFAULTSHADER_H
#include "ShaderInterface.h"

class DefaultShader : public ShaderInterface {
public:
	void DrawMesh(const BaseMesh* pMesh );
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice);
	void OnD3D11DestroyDevice();

	//Statics
private:
	static boolean sCompiled;
	static ID3D11InputLayout* sVertexLayout;
	static ID3D11VertexShader* sVertexShader;
	static ID3D11PixelShader* sPixelShader;
	static ID3D11Buffer* sConstantBuffer;

};

#endif