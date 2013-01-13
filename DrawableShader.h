#ifndef DRAWABLESHADER_H
#define DRAWABLESHADER_H

class DrawableMesh;
class DrawableState;

class DrawableShader {
public:
	WCHAR* mShaderHandle;
	virtual void DrawMesh(ID3D11DeviceContext* pd3dContext, const DrawableMesh* pMesh, const DrawableState* pState) = 0;
	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice) = 0;
	virtual void OnD3D11DestroyDevice() = 0;

	DrawableShader(const WCHAR* pShaderHandle);
	virtual ~DrawableShader();

private:
	DrawableShader(const DrawableShader& copy);
};

#endif