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

	HRESULT init(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void cleanup();
	void draw(ID3D11DeviceContext* pd3dImmediateContext);

//Static members
public:
	static boolean compiled;
private:
	static ID3D11VertexShader*     vertexShader;
	static ID3D11PixelShader*      pixelShader;
	static ID3D11InputLayout*      vertexLayout;
	static ID3D11Buffer*           vertexBuffer;
	static ID3D11Buffer*           indexBuffer;
	static ID3D11Buffer*           constantBuffer;
};

#endif