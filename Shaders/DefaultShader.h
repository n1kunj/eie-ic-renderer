#ifndef SHADERS_DEFAULTSHADER_H
#define SHADERS_DEFAULTSHADER_H
#include "DrawableShaderInterface.h"

class DefaultShader : public DrawableShaderInterface {
public:
	void DrawMesh(ID3D11DeviceContext* pd3dContext, const DrawableMesh* pMesh);
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice);
	void OnD3D11DestroyDevice();

	DefaultShader();
	~DefaultShader();

	//Statics
private:
	static boolean sCompiled;
	static ID3D11InputLayout* sVertexLayout;
	static ID3D11VertexShader* sVertexShader;
	static ID3D11PixelShader* sPixelShader;
	static ID3D11Buffer* sConstantBuffer;

};

#endif