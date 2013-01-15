#pragma once
#ifndef CAMERA_H
#define CAMERA_H
#include "DirectXMath/DirectXMath.h"

class CameraButton{
public:
	CameraButton();

	void Push();
	void Release();
	DWORD GetTicksPressedFor();

private:
	BOOLEAN mPushed;
	DWORD mDownTime;
	DWORD mUpTime;
	DWORD mLastProcessed;
};
class Camera {
public:
	DirectX::XMVECTOR mEye;
	DirectX::XMVECTOR mLookVector;
	DirectX::XMVECTOR mUp;
	DirectX::XMMATRIX mViewMatrix;
	DirectX::XMMATRIX mProjectionMatrix;
	DirectX::XMMATRIX mViewProjectionMatrix;

	void update(DXGI_SURFACE_DESC pSurfaceDesc);
	void updateWindowDimensions();

	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
		void* pUserContext );

	Camera();
	~Camera();

private:
	POINT mMouseStart;
	DOUBLE mMoveDistanceX;
	DOUBLE mMoveDistanceY;

	boolean mMouseLooking;
	boolean mMouseCentred;

	INT mScreenCentreX;
	INT mScreenCentreY;

	Camera(const Camera& camera);
	void updateCameraLook(DirectX::XMINT2 pMoveDelta);
	void updateCameraMove();
	void keyInteracted(UINT uMsg, WPARAM wParam, bool* pbNoFurtherProcessing);
	CameraButton mCamMoveForward;
	CameraButton mCamMoveBackward;
	CameraButton mCamStrafeLeft;
	CameraButton mCamStrafeRight;
	CameraButton mCamMoveDown;
	CameraButton mCamMoveUp;
};

#endif