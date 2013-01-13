#ifndef DRAWABLE_STATE_H
#define DRAWABLE_STATE_H
#include <xnamath.h>

class DrawableState {
public:
	XMMATRIX mWorldViewMatrix;
	XMMATRIX mViewMatrix;
	XMMATRIX mProjectionMatrix;

	DrawableState();

	~DrawableState();
};

#endif