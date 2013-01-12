#ifndef MESHES_CUBE_H
#define MESHES_CUBE_H
#include "..\DrawableMesh.h"
#include <xnamath.h>

class CubeMesh : public DrawableMesh {
public:

	CubeMesh();
	virtual ~CubeMesh();

	HRESULT draw(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, const DXGI_SURFACE_DESC* pSurfaceDesc);

protected:
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, const DXGI_SURFACE_DESC* pSurfaceDesc);
};

#endif