#include "DXUT.h"
#include "DrawableMesh.h"

DrawableMesh::DrawableMesh() : mVertexBuffer(NULL), mIndexBuffer(NULL), mInitialised(FALSE)
{
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
