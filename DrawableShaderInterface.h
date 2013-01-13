#ifndef DRAWABLESHADERINTERFACE_H
#define DRAWABLESHADERINTERFACE_H
#include "DrawableMesh.h"
#include "DrawableState.h"

class DrawableShaderInterface {
public:
	virtual void DrawMesh(ID3D11DeviceContext* pd3dContext, const DrawableMesh* pMesh, const DrawableState* pState) = 0;
	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice) = 0;
	virtual void OnD3D11DestroyDevice() = 0;

	DrawableShaderInterface() {};
	virtual ~DrawableShaderInterface() {};
};

#endif