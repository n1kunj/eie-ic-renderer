#ifndef MESHES_CUBE_H
#define MESHES_CUBE_H
#include "DXUT.h"
#include "Mesh.h"
#include <xnamath.h>

class CubeMesh : public BaseMesh {
public:

	CubeMesh();
	~CubeMesh();

	HRESULT draw(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, const DXGI_SURFACE_DESC* pSurfaceDesc);
	void OnD3D11DestroyDevice();

protected:
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, const DXGI_SURFACE_DESC* pSurfaceDesc);

//Static members
public:
	static boolean sInitialised;
private:
	static ID3D11VertexShader*     sVertexShader;
	static ID3D11PixelShader*      sPixelShader;
	static ID3D11InputLayout*      sVertexLayout;
	//static ID3D11Buffer*           sVertexBuffer;
	//static ID3D11Buffer*           sIndexBuffer;
	static ID3D11Buffer*           sConstantBuffer;
};

#endif