#ifndef MESHES_MESH_H
#define MESHES_MESH_H
#include <xnamath.h>

struct Vertex
{
	XMFLOAT3 POSITION;
	XMFLOAT4 COLOR;
};

class DrawableMesh {
public:
	XMMATRIX mWorldViewMatrix;
	XMMATRIX mViewMatrix;
	XMMATRIX mProjectionMatrix;

	boolean mInitialised;
	ID3D11Buffer* mVertexBuffer;
	ID3D11Buffer* mIndexBuffer;

	virtual HRESULT draw(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, const DXGI_SURFACE_DESC* pSurfaceDesc) = 0;
	virtual void OnD3D11DestroyDevice();

	DrawableMesh();

	virtual ~DrawableMesh();

protected:
	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, const DXGI_SURFACE_DESC* pSurfaceDesc) = 0;
	
};



#endif