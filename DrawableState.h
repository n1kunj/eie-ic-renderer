#pragma once
#ifndef DRAWABLE_STATE_H
#define DRAWABLE_STATE_H
#include "DirectXMath\DirectXMath.h"

class DrawableState {
public:
	DirectX::XMFLOAT3 mPosition;
	DirectX::XMFLOAT3 mRotation;

	DirectX::XMMATRIX mModelMatrix;

	boolean mDirty;

	void updateMatrices();

	DrawableState();
	~DrawableState();

private:
	DrawableState(const DrawableState& copy);
};

#endif