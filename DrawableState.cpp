#include "DXUT.h"
#include "DrawableState.h"
#include "Camera.h"
#include "Procedural/Generator.h"

using namespace DirectX;

DrawableState::DrawableState()
	: mPosition(0,0,0), mRotation(0,0,0),
	mScale(1.0f,1.0f,1.0f),	mDiffuseColour(1.0f,0.00f,0.50f),
	mSpecularColour(0.0f,1.0f,0.0f), mAmbientColour(1.0f,0.50f,0.50f),
	mSpecularExponent(100.0f),mSpecularAmount(1.0f), mCoords(0,0,0),
	mDistantTextures(NULL),//mInstanceBuffer(NULL),mIndirectBuffer(NULL)
	mCityTile(NULL)
{
	// Initialize the world matrix
	mModelMatrix = XMMatrixIdentity();
}

DrawableState::~DrawableState()
{

}

void DrawableState::updateMatrices()
{
	XMMATRIX scaleMatrix = XMMatrixScaling(mScale.x,mScale.y,mScale.z);
	XMMATRIX rotateMatrix = XMMatrixRotationRollPitchYaw(mRotation.x,mRotation.y,mRotation.z);
	XMMATRIX transformMatrix = XMMatrixMultiply(scaleMatrix,rotateMatrix);
	XMMATRIX translateMatrix = XMMatrixTranslation(mPosition.x,
		mPosition.y,mPosition.z);
	mModelMatrix = XMMatrixMultiply(transformMatrix, translateMatrix);
}

void DrawableState::setPosition( DOUBLE x, DOUBLE y, DOUBLE z )
{
	INT dX = (INT) floor(x);
	INT dY = (INT) floor(y);
	INT dZ = (INT) floor(z);
	mPosition = XMFLOAT3((FLOAT)(x - dX),(FLOAT)(y - dY),(FLOAT)(z - dZ));
	mCoords = DirectX::XMINT3(dX,dY,dZ);
}
