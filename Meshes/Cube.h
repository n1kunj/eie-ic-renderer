#ifndef MESHES_CUBE_H
#define MESHES_CUBE_H

class CubeMesh {
public:
	CubeMesh();
	~CubeMesh();

	HRESULT init(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext);
	void cleanup();
	void draw(ID3D11DeviceContext* pd3dImmediateContext);
	static boolean dirty;

private:
	static ID3D11VertexShader*     vertexShader;
	static ID3D11PixelShader*      pixelShader;
	static ID3D11InputLayout*      vertexLayout;
	static ID3D11Buffer*           vertexBuffer;

};

#endif