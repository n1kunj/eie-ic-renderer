#pragma once
#ifndef DRAWABLE_STATE_H
#define DRAWABLE_STATE_H
#include "DirectXMath\DirectXMath.h"
#include <memory>
#include "Texture2D.h"
#include "DrawableMesh.h"

class Camera;
class DistantTile;
class CityTile;

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
	FLOAT mSpecularAmount;
	std::shared_ptr<DistantTile> mDistantTile;
	std::shared_ptr<CityTile> mCityTile;

	void updateMatrices();
	void setPosition(DOUBLE x, DOUBLE y, DOUBLE z);
	FLOAT getBoundingRadius() {
		//TODO: better bounds calculations
		return 5 * mScale.x;
	}
	DOUBLE getPosX() {
		return ((DOUBLE) mPosition.x) + mCoords.x;
	}
	DOUBLE getPosY() {
		return ((DOUBLE) mPosition.y) + mCoords.y;
	}
	DOUBLE getPosZ() {
		return ((DOUBLE) mPosition.z) + mCoords.z;
	}

	DrawableState();
	~DrawableState();

private:
	DrawableState(const DrawableState& copy);
};

#endif