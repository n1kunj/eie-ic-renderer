#pragma once
#ifndef DRAWABLE_MANAGER_H
#define DRAWABLE_MANAGER_H
#include <vector>
#include "Drawable.h"

class Camera;

struct OctreeStruct {
	Camera* mCamera;
};

class DrawableManager {
public:
	DrawableManager();
	~DrawableManager();
	void addDrawable(Drawable* pDrawable);
	void Draw(ID3D11DeviceContext* pd3dContext);
	BOOLEAN removeDrawable( Drawable* pDrawable, BOOLEAN pDelete);
	void reset();
private:
	std::vector<Drawable*> mDrawableVector;
};

#endif // !DRAWABLE_MANAGER_H
