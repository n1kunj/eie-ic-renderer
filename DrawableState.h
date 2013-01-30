#pragma once
#ifndef DRAWABLE_STATE_H
#define DRAWABLE_STATE_H
#include "DirectXMath\DirectXMath.h"

class Camera;

class DrawableState {
public:
	DirectX::XMMATRIX mModelMatrix;
	DirectX::XMFLOAT3 mPosition;
	DirectX::XMFLOAT3 mRotation;
	DirectX::XMFLOAT3 mScale;
	DirectX::XMFLOAT3 mDiffuseColour;
	DirectX::XMFLOAT3 mAmbientColour;
	DirectX::XMFLOAT3 mSpecularColour;
	DirectX::XMINT3 mCoords;
	DirectX::XMINT3 mCamOffset;
	FLOAT mSpecularExponent;

	void updateMatrices(Camera* pCamera);
	void setPosition(DOUBLE x, DOUBLE y, DOUBLE z);

	DrawableState();
	~DrawableState();

private:
	DrawableState(const DrawableState& copy);
};

#endif