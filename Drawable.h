#pragma once
#ifndef DRAWABLE_H
#define DRAWABLE_H
#include "DrawableState.h"

class DrawableMesh;
class DrawableShader;
class Camera;

class Drawable {
public:
	virtual void Draw(ID3D11DeviceContext* pd3dContext) = 0;
	virtual ~Drawable() {};
};

class BasicDrawable : public Drawable {
public:
	DrawableState mState;
	DrawableMesh* mMesh;
	DrawableShader* mShader;
	Camera* mCamera;

	void Draw(ID3D11DeviceContext* pd3dContext);
	
	BasicDrawable(DrawableMesh* pMesh, DrawableShader* pShader, Camera* pCamera);
	BasicDrawable(const BasicDrawable& copy);
	~BasicDrawable();
};

#endif