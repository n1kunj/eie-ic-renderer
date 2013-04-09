#pragma once
#ifndef DRAWABLE_H
#define DRAWABLE_H
#include "DrawableState.h"

class DrawableMesh;
class DrawableShader;
class Camera;

class Drawable {
public:
	Drawable(Camera* pCamera) : mCamera(pCamera) {};
	virtual void Draw(ID3D11DeviceContext* pd3dContext) = 0;
	virtual ~Drawable() {};
	Camera* mCamera;
};

class BasicDrawable : public Drawable {
public:
	DrawableState mState;
	DrawableMesh* mMesh;
	DrawableShader* mShader;

	void Draw(ID3D11DeviceContext* pd3dContext);
	
	BasicDrawable(DrawableMesh* pMesh, DrawableShader* pShader, Camera* pCamera);
	BasicDrawable(const BasicDrawable& copy);
	virtual ~BasicDrawable();
};

#endif