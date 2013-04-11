#pragma once
#ifndef CAMERA_H
#define CAMERA_H
#include "DirectXMath/DirectXMath.h"

class CameraButton{
public:
	CameraButton();

	void Push();
	void Release();
	double GetTicksPressedFor();

private:
	BOOLEAN mPushed;
	double mDownTime;
	double mUpTime;
	double mLastProcessed;
};
class Camera {
public:
	DirectX::XMVECTOR mEye;
	DirectX::XMVECTOR mLookVector;
	DirectX::XMVECTOR mUp;
	DirectX::XMMATRIX mViewMatrix;
	DirectX::XMMATRIX mProjectionMatrix;
	DirectX::XMMATRIX mViewProjectionMatrix;
	DirectX::XMINT3 mCoords;
	FLOAT mzFar;
	FLOAT mzNear;
	FLOAT mYFOV;
	DirectX::XMFLOAT2 padding0;

	void update(DXGI_SURFACE_DESC pSurfaceDesc);
	void updateWindowDimensions();
	BOOL testFrustum( DirectX::XMFLOAT3 pPos, DirectX::XMINT3 pCoords, FLOAT pSphereRadius ) const;

	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
		void* pUserContext );

	void setEye(DOUBLE x, DOUBLE y, DOUBLE z);

	DOUBLE getEyeX() {
		return ((DOUBLE) DirectX::XMVectorGetX(mEye)) + mCoords.x;
	}
	DOUBLE getEyeY() {
		return ((DOUBLE) DirectX::XMVectorGetY(mEye)) + mCoords.y;
	}
	DOUBLE getEyeZ() {
		return ((DOUBLE) DirectX::XMVectorGetZ(mEye)) + mCoords.z;
	}

	Camera();
	~Camera();

private:
	DirectX::XMVECTOR mActualUp;

	DirectX::XMVECTOR mNearNormal;
	DirectX::XMVECTOR mFarNormal;
	DirectX::XMVECTOR mLeftNormal;
	DirectX::XMVECTOR mRightNormal;
	DirectX::XMVECTOR mTopNormal;
	DirectX::XMVECTOR mBottomNormal;

	DirectX::XMVECTOR mLeftTopNearPoint;
	DirectX::XMVECTOR mRightBottomFarPoint;

	POINT mMouseStart;
	DOUBLE mMoveDistanceX;
	DOUBLE mMoveDistanceY;

	boolean mForceMouseLooking;
	boolean mHeldMouseLooking;
	boolean mMouseCentred;

	INT mScreenCentreX;
	INT mScreenCentreY;

	Camera(const Camera& camera);
	void updateCameraLook(DirectX::XMINT2 pMoveDelta);
	void updateCameraMove();
	void keyInteracted(HWND hWnd, UINT uMsg, WPARAM wParam, bool* pbNoFurtherProcessing);
	void MouseLookEnabled(HWND hWnd);
	void MouseLookDisabled();
	CameraButton mCamMoveForward;
	CameraButton mCamMoveBackward;
	CameraButton mCamStrafeLeft;
	CameraButton mCamStrafeRight;
	CameraButton mCamMoveDown;
	CameraButton mCamMoveUp;
};

#endif