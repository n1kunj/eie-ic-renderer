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
	Drawable(const Drawable& copy);
protected:
	Camera* mCamera;
private:
};

class BasicDrawable : public Drawable {
public:
	DrawableState mState;
	DrawableMesh* mMesh;
	DrawableShader* mShader;

	void Draw(ID3D11DeviceContext* pd3dContext);
	static void DrawInstanced(ID3D11DeviceContext* pd3dContext, BasicDrawable* pDrawableList, UINT pCount);
	
	BasicDrawable(DrawableMesh* pMesh, DrawableShader* pShader, Camera* pCamera);
	virtual ~BasicDrawable();
	BasicDrawable(const BasicDrawable& copy);
};

#endif