#include "DXUT.h"
#pragma once
#ifndef MESH_MANAGER_H
#define MESH_MANAGER_H
#include <vector>
#include "DrawableMesh.h"
#include <sstream>

class MeshManager{
private:
	std::vector<DrawableMesh*> mDrawableMeshes;
public:
	MeshManager() {}
	//Memory now belongs to the shader manager. It'll delete as required
	void addMesh(DrawableMesh* pMesh) {
		mDrawableMeshes.push_back(pMesh);
	}
	DrawableMesh* getDrawableMesh(std::string pMeshName) {
		std::wstring ws(pMeshName.begin(),pMeshName.end());
		for (int i = 0; i < mDrawableMeshes.size(); i++) {
			if (mDrawableMeshes[i]->mModelHandle == ws) {
				return mDrawableMeshes[i];
			}
		}
		std::stringstream ss;
		ss << "Drawable Mesh " << ws.c_str() << " Does not exist!"; 
		throw std::exception(ss.str().c_str());
	}

	void OnD3D11CreateDevice( ID3D11Device* pd3dDevice ) 
	{
		for (int i = 0; i < mDrawableMeshes.size(); i++) {
			mDrawableMeshes[i]->OnD3D11CreateDevice(pd3dDevice);
		}
	}

	void OnD3D11DestroyDevice() 
	{
		for (int i = 0; i < mDrawableMeshes.size(); i++) {
			mDrawableMeshes[i]->OnD3D11DestroyDevice();
		}
	}

	~MeshManager() {
		for (int i = 0; i < mDrawableMeshes.size(); i++) {
			delete mDrawableMeshes[i];
		}
	}
};




#endif