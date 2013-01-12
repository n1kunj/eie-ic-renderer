#ifndef MESHES_MESH_H
#define MESHES_MESH_H
#include <xnamath.h>

class BaseMesh {
public:
	XMMATRIX mWorldViewMatrix;
	XMMATRIX mViewMatrix;
	XMMATRIX mProjectionMatrix;

	ID3D11Buffer* mVertexBuffer;
	ID3D11Buffer* mIndexBuffer;

	virtual HRESULT draw(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, const DXGI_SURFACE_DESC* pSurfaceDesc) = 0;
	virtual void OnD3D11DestroyDevice() = 0;

	BaseMesh();

	virtual ~BaseMesh();

protected:
	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, const DXGI_SURFACE_DESC* pSurfaceDesc) = 0;
	
};



#endif