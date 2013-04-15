#pragma once
#ifndef DRAWABLESHADER_H
#define DRAWABLESHADER_H
#include <string>

class DrawableMesh;
class DrawableState;
class Camera;

class DrawableShader {
public:
	std::wstring mShaderHandle;
	virtual void DrawMesh(ID3D11DeviceContext* pd3dContext, const DrawableMesh* pMesh, const DrawableState* pState, const Camera* pCamera) = 0;
	virtual void DrawInstanced(ID3D11DeviceContext* pd3dContext, const DrawableMesh* pMesh, DrawableState* const* pState, const Camera* pCamera, size_t pCount) {
		for (int i = 0; i < pCount; i++) {
			DrawMesh(pd3dContext,pMesh,pState[i],pCamera);
		}
	}
	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice) = 0;
	virtual void OnD3D11DestroyDevice() = 0;

	DrawableShader(const WCHAR* pShaderHandle) {
		mShaderHandle = std::wstring(pShaderHandle);
	};
	virtual ~DrawableShader() {
	};

private:
	DrawableShader(const DrawableShader& copy);
};

#endif