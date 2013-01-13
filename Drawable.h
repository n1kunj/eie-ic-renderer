#ifndef DRAWABLE_H
#define DRAWABLE_H
#include "DrawableState.h"

class DrawableMesh;
class DrawableShader;

class Drawable {
public:
	DrawableState mState;
	DrawableMesh* mMesh;
	DrawableShader* mShader;

	void Draw(ID3D11DeviceContext* pd3dContext);
	
	Drawable(DrawableMesh* pMesh, DrawableShader* pShader);
	~Drawable();

private:
	Drawable(const Drawable& copy);
};

#endif