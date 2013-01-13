#include "DXUT.h"
#include "CubeMeshLoader.h"

Vertex vertices[] =
{
	{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
	{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
	{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ) },
	{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) },
	{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 0.0f, 1.0f, 1.0f ) },
	{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ) },
	{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ) },
	{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f ) },
};

UINT indices[] =
{
	3,1,0,
	2,1,3,

	0,5,4,
	1,5,0,

	3,4,7,
	0,4,3,

	1,6,5,
	2,6,1,

	2,7,6,
	3,7,2,

	6,4,5,
	7,4,6,
};

Vertex* CubeMeshLoader::loadVertices( UINT* retNumVertices )
{
	*retNumVertices = (UINT) sizeof(vertices)/sizeof(Vertex);
	return vertices;
}

UINT* CubeMeshLoader::loadIndices( UINT* retNumIndices )
{
	*retNumIndices = (UINT) sizeof(indices)/sizeof(UINT);
	return indices;
}

void CubeMeshLoader::cleanup()
{

}

CubeMeshLoader::CubeMeshLoader()
{

}

CubeMeshLoader::~CubeMeshLoader()
{
	cleanup();
}
