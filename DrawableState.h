#pragma once
#ifndef DRAWABLE_STATE_H
#define DRAWABLE_STATE_H
#include <xnamath.h>

class DrawableState {
public:
	XMFLOAT3 mPosition;
	XMFLOAT3 mRotation;

	XMMATRIX mModelMatrix;

	boolean mDirty;

	void updateMatrices();

	DrawableState();
	~DrawableState();

private:
	DrawableState(const DrawableState& copy);
};

#endif