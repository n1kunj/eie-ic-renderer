#include "DXUT.h"
#include "DrawableManager.h"

DrawableManager::DrawableManager() : mDrawableVector()
{
	mDrawableVector.reserve(100000);
}

DrawableManager::~DrawableManager()
{
	reset();
}

void DrawableManager::addDrawable( boost::shared_ptr<Drawable> pDrawable )
{
	mDrawableVector.push_back(pDrawable);
}

//Return TRUE if successfully removed, else FALSE
BOOLEAN DrawableManager::removeDrawable( boost::shared_ptr<Drawable> pDrawable)
{
	for (int i = 0; i < mDrawableVector.size(); i++) {
		if (mDrawableVector[i] == pDrawable) {
			mDrawableVector[i].reset();
			return TRUE;
		}
	}
	return FALSE;
}

void DrawableManager::Draw(ID3D11DeviceContext* pd3dContext)
{
	for (UINT i = 0; i < mDrawableVector.size(); i++) {
		Drawable* d = mDrawableVector[i].get();
		if (d!=NULL) {
			d->Draw(pd3dContext);
		}
	}
}

void DrawableManager::reset()
{
	for (UINT i = 0; i < mDrawableVector.size(); i++) {
		mDrawableVector[i].reset();
	}
	mDrawableVector.clear();
}
