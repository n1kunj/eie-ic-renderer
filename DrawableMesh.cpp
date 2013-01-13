#include "DXUT.h"
#include "DrawableMesh.h"

DrawableMesh::DrawableMesh(const WCHAR* pModelHandle, MeshLoaderInterface* pMeshLoader) : 
	mMeshLoader(pMeshLoader), mVertexBuffer(NULL), mIndexBuffer(NULL), mInitialised(FALSE), mIndexBufferFormat(DXGI_FORMAT_R32_UINT)
{
	size_t linelen = wcslen(pModelHandle);
	mModelHandle = new WCHAR[linelen+1];
	wcscpy_s(mModelHandle,linelen+1,pModelHandle);
}

DrawableMesh::~DrawableMesh()
{
	OnD3D11DestroyDevice();
}

void DrawableMesh::OnD3D11DestroyDevice()
{
	SAFE_RELEASE(mVertexBuffer);
	SAFE_RELEASE(mIndexBuffer);
}

HRESULT DrawableMesh::OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
{
	if (mInitialised) {
		return S_OK;
	}

	HRESULT hr = S_OK;

	//Load vertices buffer
	Vertex* vertices = mMeshLoader->loadVertices(&mNumVertices);

	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.ByteWidth = sizeof(Vertex) * mNumVertices;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = vertices;
	V_RETURN(pd3dDevice->CreateBuffer( &bd, &InitData, &mVertexBuffer ));

	//Load indices buffer
	UINT* indices = mMeshLoader->loadIndices(&mNumIndices);

	bd.ByteWidth = sizeof(UINT) * mNumIndices;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	V_RETURN(hr = pd3dDevice->CreateBuffer( &bd, &InitData, &mIndexBuffer ));

	mInitialised = true;
	return S_OK;
}
