#ifndef MESHES_CUBE_H
#define MESHES_CUBE_H
#include <xnamath.h>

class CubeMesh {
public:
	XMMATRIX worldViewMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;

	CubeMesh();
	~CubeMesh();

	HRESULT draw(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, const DXGI_SURFACE_DESC* surfaceDesc);
	void cleanup();

private:
	HRESULT init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, const DXGI_SURFACE_DESC* surfaceDesc);

//Static members
public:
	static boolean initialised;
private:
	static ID3D11VertexShader*     vertexShader;
	static ID3D11PixelShader*      pixelShader;
	static ID3D11InputLayout*      vertexLayout;
	static ID3D11Buffer*           vertexBuffer;
	static ID3D11Buffer*           indexBuffer;
	static ID3D11Buffer*           constantBuffer;
};

#endif