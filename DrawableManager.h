#pragma once
#ifndef DRAWABLE_MANAGER_H
#define DRAWABLE_MANAGER_H
#include <vector>
#include "Drawable.h"
#include <boost/shared_ptr.hpp>

class Camera;

class DrawableManager {
private:
	std::vector<boost::shared_ptr<Drawable> > mDrawableVector;
public:
	DrawableManager();
	~DrawableManager();
	void addDrawable(boost::shared_ptr<Drawable> pDrawable);
	BOOLEAN DrawableManager::removeDrawable( boost::shared_ptr<Drawable> pDrawable);
	void Draw(ID3D11DeviceContext* pd3dContext);
	void reset();
};

#endif // !DRAWABLE_MANAGER_H
