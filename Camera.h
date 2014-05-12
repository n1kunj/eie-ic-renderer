#pragma once
#ifndef CAMERA_H
#define CAMERA_H
#include "DirectXMath/DirectXMath.h"

struct RendererSettings;

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
	BOOL testFrustumAABB( DirectX::XMFLOAT3 pPos, DirectX::XMINT3 pCoords, DirectX::XMFLOAT3 pLenXYZ ) const;

	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
		void* pUserContext );

	void setEye(DOUBLE x, DOUBLE y, DOUBLE z);

	DOUBLE getEyeX() const {
		return ((DOUBLE) DirectX::XMVectorGetX(mEye)) + mCoords.x;
	}
	DOUBLE getEyeY() const {
		return ((DOUBLE) DirectX::XMVectorGetY(mEye)) + mCoords.y;
	}
	DOUBLE getEyeZ() const {
		return ((DOUBLE) DirectX::XMVectorGetZ(mEye)) + mCoords.z;
	}

	void smoothMove(DOUBLE x, DOUBLE y, DOUBLE z, DOUBLE speed);

	void constMove(DOUBLE speedX, DOUBLE speedY, DOUBLE speedZ);

	void setLook(DOUBLE zenith, DOUBLE azimuth);

	void smoothLook(DOUBLE zenith, DOUBLE azimuth, DOUBLE rotSpeed);

	void constLook(DOUBLE dz, DOUBLE da);

	Camera(RendererSettings* pRendererSettings);
	~Camera();

private:
	DirectX::XMVECTOR mActualUp;

	DirectX::XMVECTOR mNearPlane;
	DirectX::XMVECTOR mFarPlane;
	DirectX::XMVECTOR mLeftPlane;
	DirectX::XMVECTOR mRightPlane;
	DirectX::XMVECTOR mTopPlane;
	DirectX::XMVECTOR mBottomPlane;

	DirectX::XMVECTOR mNearPlaneAbs;
	DirectX::XMVECTOR mFarPlaneAbs;
	DirectX::XMVECTOR mLeftPlaneAbs;
	DirectX::XMVECTOR mRightPlaneAbs;
	DirectX::XMVECTOR mTopPlaneAbs;
	DirectX::XMVECTOR mBottomPlaneAbs;

	DirectX::XMVECTOR mLeftTopNearPoint;
	DirectX::XMVECTOR mRightBottomFarPoint;

	DOUBLE mLastUpdateTime;

	POINT mMouseStart;
	DOUBLE mLookDistanceX;
	DOUBLE mLookDistanceY;

	BOOL mSmoothLook;
	DOUBLE mSmoothLookDistanceX;
	DOUBLE mSmoothLookDistanceY;
	DOUBLE mRotSpeed;

	BOOL mConstLook;
	DOUBLE mConstLookDX;
	DOUBLE mConstLookDY;

	DOUBLE mCameraMoveSpeed;

	BOOL mSmoothMove;
	DOUBLE mSmoothMoveX;
	DOUBLE mSmoothMoveY;
	DOUBLE mSmoothMoveZ;
	DOUBLE mSmoothMoveSpeed;

	BOOL mConstMove;
	DOUBLE mConstMoveX;
	DOUBLE mConstMoveY;
	DOUBLE mConstMoveZ;

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
	RendererSettings* mRendererSettings;
	
};

#endif