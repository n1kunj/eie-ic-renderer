#pragma once
#ifndef DRAWABLE_MANAGER_H
#define DRAWABLE_MANAGER_H
#include <vector>
#include "Drawable.h"
#include <boost/shared_ptr.hpp>

class Camera;

struct OctreeStruct {
	Camera* mCamera;
};

class DrawableManager {
public:
	DrawableManager();
	~DrawableManager();
	void addDrawable(boost::shared_ptr<Drawable> pDrawable);
	void Draw(ID3D11DeviceContext* pd3dContext);
	BOOLEAN DrawableManager::removeDrawable( boost::shared_ptr<Drawable> pDrawable);
	void reset();
private:
	std::vector<boost::shared_ptr<Drawable> > mDrawableVector;
};

#endif // !DRAWABLE_MANAGER_H
