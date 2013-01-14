#pragma once
#ifndef MESHES_CUBE_H
#define MESHES_CUBE_H
#include "..\DrawableMesh.h"

class CubeMeshLoader : public MeshLoaderInterface {
public:

	CubeMeshLoader();
	~CubeMeshLoader();

	Vertex* loadVertices( UINT* retNumVertices );
	UINT* loadIndices( UINT* retNumIndices );

	void cleanup();

};

#endif