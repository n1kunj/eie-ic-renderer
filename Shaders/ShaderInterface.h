#ifndef SHADERS_SHADERBASE_H
#define SHADERS_SHADERBASE_H
#include "../Meshes/Mesh.h"

class ShaderInterface {
public:
	virtual void DrawMesh(const BaseMesh* pMesh) = 0;
	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice) = 0;
	virtual void OnD3D11DestroyDevice() = 0;
};

#endif