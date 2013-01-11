#ifndef MESHES_MESH_H
#define MESHES_MESH_H
#include <xnamath.h>

class BaseMesh {
public:
	XMMATRIX mWorldViewMatrix;
	XMMATRIX mViewMatrix;
	XMMATRIX mProjectionMatrix;

	virtual HRESULT draw(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, const DXGI_SURFACE_DESC* pSurfaceDesc) = 0;
	virtual void cleanup() = 0;

protected:
	virtual HRESULT init(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, const DXGI_SURFACE_DESC* pSurfaceDesc) = 0;
	
};



#endif