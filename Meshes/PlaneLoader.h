#include "DXUT.h"
#pragma once
#ifndef MESHES_PLANE_LOADER_H
#define MESHES_PLANE_LOADER_H
#include "..\DrawableMesh.h"

class PlaneLoader : public MeshLoaderInterface {
private:
	UINT mSideVertCount;
	UINT mNumVertices;
	UINT mNumIndices;
	Vertex* mVerts;
	UINT* mIndices;
public:
	//Creates a plane with the specified vert count on each side.
	//From -0.5,0,-0.5 to 0.5,0,0.5
	PlaneLoader(UINT pSideVertCount) : mSideVertCount(max(2,pSideVertCount)),
		mVerts(NULL),mIndices(NULL) {
		mNumVertices = mSideVertCount * mSideVertCount;
		UINT numTris = mSideVertCount - 1;
		numTris = 2 * numTris * numTris;
		mNumIndices = 3 * numTris;
	}


	~PlaneLoader() {
		cleanup();
	}

	Vertex* loadVertices( UINT* retNumVertices ) {
		if (mVerts == NULL) {
			createPlane();
		}
		*retNumVertices = mNumVertices;
		return mVerts;

	}
	UINT* loadIndices( UINT* retNumIndices ) {
		if (mIndices == NULL) {
			createPlane();
		}
		*retNumIndices = mNumIndices;

		return mIndices;
	}

	void cleanup() {
		SAFE_DELETE(mVerts);
		SAFE_DELETE(mIndices);
	}
private:
	void createPlane() {
		using namespace DirectX;

		mVerts = new Vertex[mNumVertices];
		mIndices = new UINT[mNumIndices];

		//Create from 0,0,0 to 1,0,1 then shift
		for (UINT i = 0; i < mNumVertices; i++) {
			Vertex &vert = mVerts[i];
			float x = float(i % mSideVertCount)/(mSideVertCount-1) - 0.5f;
			float z = float(i / mSideVertCount)/(mSideVertCount-1) - 0.5f;
			vert.POSITION = XMFLOAT3(x,0,z);
			vert.NORMAL = XMFLOAT3(0,1.0f,0);
		}

		for (UINT i = 0; i < mNumIndices/6; i++) {
			UINT *ind = &mIndices[i*6];

			UINT bl = i + i/(mSideVertCount-1);
			UINT br = bl + 1;
			UINT tl = bl + mSideVertCount;
			UINT tr = tl + 1;

			ind[0] = br;
			ind[1] = bl;
			ind[2] = tl;
			
			ind[3] = br;
			ind[4] = tl;
			ind[5] = tr;
		}
	}
};

#endif