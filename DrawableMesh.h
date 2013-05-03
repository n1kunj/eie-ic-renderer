#pragma once
#ifndef DRAWABLE_MESH_H
#define DRAWABLE_MESH_H
#include "DirectXMath\DirectXMath.h"
#include <string>

class MeshLoaderInterface;

struct VertexData
{
	DirectX::XMFLOAT3 POSITION;
	DirectX::XMFLOAT3 NORMAL;
};

const UINT VertexDataStride = sizeof(VertexData);

struct InstanceData
{
	DirectX::XMFLOAT3 POSITION;
};

//const UINT InstanceDataStride = sizeof(InstanceData);

const D3D11_INPUT_ELEMENT_DESC VertexLayout[] =
{
	//Vertex Data, slot 0
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//Instance Data, slot 1
	//{ "POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 0 },
};

const UINT numLayoutElements = ARRAYSIZE(VertexLayout);

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

	//The loader instance now belongs to me! I'll make sure it's deleted when I'm done with it
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
	virtual VertexData* loadVertices(UINT* retNumVertices) = 0;

	/**Returns a dynamically allocated array of indices
	Remains allocated until you call cleanup or destroy the object
	Pass in a UINT pointer to get number of indices returned*/
	virtual UINT* loadIndices(UINT* retNumIndices) = 0;

	virtual void cleanup() = 0;

	MeshLoaderInterface() {};
	virtual ~MeshLoaderInterface() {};
};

#endif