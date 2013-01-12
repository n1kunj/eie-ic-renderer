#include "DXUT.h"
#include "Mesh.h"

BaseMesh::BaseMesh()
{
	mVertexBuffer = NULL;
	mIndexBuffer = NULL;
}

BaseMesh::~BaseMesh()
{
	OnD3D11DestroyDevice();
}

void BaseMesh::OnD3D11DestroyDevice()
{
	SAFE_RELEASE(mVertexBuffer);
	SAFE_RELEASE(mIndexBuffer);
}
