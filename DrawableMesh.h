#pragma once
#ifndef DRAWABLE_MESH_H
#define DRAWABLE_MESH_H
#include "DirectXMath\DirectXMath.h"
#include <string>

class MeshLoaderInterface;

struct Vertex
{
	DirectX::XMFLOAT3 POSITION;
	DirectX::XMFLOAT4 COLOR;
};

const D3D11_INPUT_ELEMENT_DESC vertexLayout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

const UINT numLayoutElements = ARRAYSIZE(vertexLayout);

class DrawableMesh {
public:
	WCHAR* mModelHandle;
	ID3D11Buffer* mVertexBuffer;
	ID3D11Buffer* mIndexBuffer;
	UINT mNumVertices;
	UINT mNumIndices;
	DXGI_FORMAT mIndexBufferFormat;
	boolean mInitialised;

	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice);
	virtual void OnD3D11DestroyDevice();

	DrawableMesh(const WCHAR* pModelHandle, MeshLoaderInterface* pMeshLoader);
	virtual ~DrawableMesh();

private:
	MeshLoaderInterface* mMeshLoader;
};

class MeshLoaderInterface {
public:
	/**Returns a dynamically allocated array of type Vertex
	Remains allocated until you call cleanup or destroy the object
	Pass in a UINT pointer to get number of verts returned**/
	virtual Vertex* loadVertices(UINT* retNumVertices) = 0;

	/**Returns a dynamically allocated array of indices
	Remains allocated until you call cleanup or destroy the object
	Pass in a UINT pointer to get number of indices returned*/
	virtual UINT* loadIndices(UINT* retNumIndices) = 0;

	virtual void cleanup() = 0;

	MeshLoaderInterface() {};
	virtual ~MeshLoaderInterface() {};
};

#endif