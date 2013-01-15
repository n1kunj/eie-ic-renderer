#include "DXUT.h"
#include "Camera.h"
#include <windowsx.h>
#define _USE_MATH_DEFINES
#include <math.h>

#define CAMERALOOKSCALE 0.005

#define CAMERAMOVESPEED (2.0/1000.0)

using namespace DirectX;

Camera::Camera() : mouseLook(FALSE), mMouseStart(0,0), mMouseEnd(0,0), mMoveDistanceX(0), mMoveDistanceY(0),
	mCamMoveBackward(),mCamMoveForward(),mCamStrafeLeft(),mCamStrafeRight(),mCamMoveUp(),mCamMoveDown()
{
	// Initialize the view matrix
	mEye = XMVectorSet( 0.0f, 2.0f, 5.0f, 0.0f );
	mLookVector = XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f );
	mUp = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
}

Camera::~Camera()
{
}

void Camera::update(DXGI_SURFACE_DESC pSurfaceDesc)
{
	updateCameraMove();

	XMVECTOR lookAt = XMVectorAdd(mEye,mLookVector);

	mViewMatrix = XMMatrixLookAtLH( mEye, lookAt, mUp );

	mProjectionMatrix = XMMatrixPerspectiveFovLH( XM_PIDIV2, pSurfaceDesc.Width / (FLOAT)pSurfaceDesc.Height, 0.01f, 100.0f );

	mViewProjectionMatrix = XMMatrixMultiply(mViewMatrix,mProjectionMatrix);
}

LRESULT Camera::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		{
			mouseLook = TRUE;
			SetCapture( hWnd );
			mMouseStart = XMINT2(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
			*pbNoFurtherProcessing = TRUE;
			break;
		}
	case WM_LBUTTONUP:
		{
			if (mouseLook == TRUE) {
				ReleaseCapture();
				mouseLook = FALSE;
				*pbNoFurtherProcessing = TRUE;
			}
			break;
		}
	case WM_MOUSEMOVE:
		{
			if (mouseLook == TRUE) {
				mMouseEnd = XMINT2(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
				XMINT2 mouseDelta = XMINT2(mMouseStart.x - mMouseEnd.x, mMouseEnd.y - mMouseStart.y);
				mMouseStart = mMouseEnd;
				mMouseEnd = XMINT2(0,0);

				updateCameraLook(mouseDelta);
				*pbNoFurtherProcessing = TRUE;
			}
			break;
		}
	case WM_CAPTURECHANGED:
		{
			mouseLook = FALSE;
			break;
		}
	case WM_KEYDOWN:
	case WM_KEYUP:
		{
			keyInteracted(uMsg, wParam, pbNoFurtherProcessing);
			break;
		}
	}

	return 0;
}

void Camera::updateCameraMove()
{
	DOUBLE forwardDist = (DOUBLE) mCamMoveForward.GetTicksPressedFor() * CAMERAMOVESPEED;
	DOUBLE backwardDist = (DOUBLE) mCamMoveBackward.GetTicksPressedFor() * CAMERAMOVESPEED;
	DOUBLE leftDist = (DOUBLE) mCamStrafeLeft.GetTicksPressedFor() * CAMERAMOVESPEED;
	DOUBLE rightDist = (DOUBLE) mCamStrafeRight.GetTicksPressedFor() * CAMERAMOVESPEED;
	DOUBLE upDist = (DOUBLE) mCamMoveUp.GetTicksPressedFor() * CAMERAMOVESPEED;
	DOUBLE downDist = (DOUBLE) mCamMoveDown.GetTicksPressedFor() * CAMERAMOVESPEED;
	
	DOUBLE deltaForward = forwardDist - backwardDist;
	DOUBLE deltaLeft = leftDist - rightDist;
	DOUBLE deltaUp = upDist - downDist;

	DOUBLE lookX = XMVectorGetX(mLookVector);
	DOUBLE lookY = XMVectorGetY(mLookVector);
	DOUBLE lookZ = XMVectorGetZ(mLookVector);

	DOUBLE dX = deltaForward * lookX + deltaLeft * -lookZ;
	DOUBLE dY = deltaForward * lookY + deltaUp;
	DOUBLE dZ = deltaForward * lookZ + deltaLeft * lookX;

	XMVECTOR upDelta = XMVectorSet((FLOAT) dX,(FLOAT) dY,(FLOAT) dZ, 0.0f);

	mEye += upDelta;

}

void Camera::updateCameraLook(XMINT2 pMoveDelta) {

		mMoveDistanceX += pMoveDelta.x;
		mMoveDistanceY += pMoveDelta.y;

		if (mMoveDistanceX > 2 * M_PI / CAMERALOOKSCALE) {
			mMoveDistanceX-= 2 * M_PI / CAMERALOOKSCALE;
		}
		else if (mMoveDistanceX < -2 * M_PI / CAMERALOOKSCALE) {
			mMoveDistanceX+= 2 * M_PI / CAMERALOOKSCALE;
		}

		//Set X axis vars
		DOUBLE curX = CAMERALOOKSCALE * mMoveDistanceX;
		DOUBLE sinX = sin(curX);
		DOUBLE cosX = cos(curX);

		//Set Y axis vars
		DOUBLE curY = CAMERALOOKSCALE * mMoveDistanceY;
		DOUBLE sinY;
		DOUBLE cosY;

		//Nik's super awesome FPS style camera. Won't let you look upside down.
		if (curY >= M_PI_2)
		{
			mMoveDistanceY = M_PI_2 / CAMERALOOKSCALE;

			sinY = 1;
			cosY = 0.0001f;
		}
		else if (curY <= -M_PI_2)
		{
			mMoveDistanceY = -M_PI_2 / CAMERALOOKSCALE;
			sinY = -1;
			cosY = 0.0001f;
		}
		else {
			sinY = sin(curY);
			cosY = cos(curY);
		}

		//Look angle (really a unit vector)
		DOUBLE angleX = sinX * cosY;
		DOUBLE angleY = -sinY;
		DOUBLE angleZ = -cosX * cosY;
		DOUBLE angleMag = sqrt(angleX * angleX + angleY * angleY + angleZ * angleZ);
		angleX /= angleMag;
		angleY /= angleMag;
		angleZ /= angleMag;

		mLookVector = XMVectorSet((FLOAT)angleX,(FLOAT)angleY,(FLOAT)angleZ,0.0f);
}

void Camera::keyInteracted(UINT uMsg, WPARAM wParam, bool* pbNoFurtherProcessing)
{
	CameraButton* button;
	switch( wParam ) {
	case 'W': 
		{
			button = &mCamMoveForward;
			break;
		}
	case 'S':
		{
			button = &mCamMoveBackward;
			break;
		}
	case 'A':
		{
			button = &mCamStrafeLeft;
			break;
		}
	case 'D':
		{
			button = &mCamStrafeRight;
			break;
		}
	case 'Q':
		{
			button = &mCamMoveUp;
			break;
		}
	case 'E':
		{
			button = &mCamMoveDown;
			break;
		}
	default:
		{
			return;
		}
	}

	if (uMsg == WM_KEYDOWN) {
		button->Push();
	}
	else {
		button->Release();
	}
}

CameraButton::CameraButton() : mPushed(FALSE),mDownTime(0),mUpTime(0),mLastProcessed(0) {
}

void CameraButton::Push()
{
	if (mPushed == TRUE) {
		return;
	}

	DWORD currentTick = GetTickCount();

	if (mLastProcessed >= mUpTime) {
		mDownTime = currentTick;
	}
	//If unprocessed since last key release event, it pretends
	//that the button was pushed down earlier
	else {
		DWORD unprocessedTime = mUpTime - mLastProcessed;
		mDownTime = currentTick - unprocessedTime;
	}

	mPushed = TRUE;
}

void CameraButton::Release()
{
	mUpTime = GetTickCount();
	mPushed = FALSE;
}

DWORD CameraButton::GetTicksPressedFor()
{
	DWORD curTime = GetTickCount();
	DWORD retTime;

	if (mPushed) {
		retTime =  curTime - max(mDownTime,mLastProcessed);
	}
	else {
		if (mLastProcessed >= mUpTime ) {
			retTime = 0;
		}
		else {
			retTime = mUpTime - mLastProcessed;
		}
	}
	mLastProcessed = curTime;

	return retTime;
}
