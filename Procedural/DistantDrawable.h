#include "DXUT.h"
#pragma once
#ifndef DISTANT_DRAWABLE_H
#define DISTANT_DRAWABLE_H
#include "..\Drawable.h"
#include <vector>

class ShaderManager;
class MeshManager;

class DistantDrawable : public Drawable {
public:
	DistantDrawable( Camera* pCamera, ShaderManager* pShaderManager, MeshManager* pMeshManager );
	void Draw(ID3D11DeviceContext* pd3dContext);
	virtual ~DistantDrawable();
private:
	DistantDrawable(const DistantDrawable& copy);
	DrawableShader* mShader;
	std::vector<BasicDrawable> mDrawables;
};


#endif