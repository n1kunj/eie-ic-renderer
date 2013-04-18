#include "DXUT.h"
#include "Camera.h"
#include <windowsx.h>
#include <WinUser.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <sstream>

#define CAMERALOOKSCALE 0.005

#define CAMERAMOVESPEED ((2.0/1000.0)*100.0)

using namespace DirectX;

double getTime() {
	return 1000 * DXUTGetTime();
}

Camera::Camera() : mHeldMouseLooking(FALSE),mMouseCentred(FALSE),
	mForceMouseLooking(FALSE), mMouseStart(), mMoveDistanceX(0),
	mMoveDistanceY(0),mCamMoveBackward(),mCamMoveForward(),
	mCamStrafeLeft(),mCamStrafeRight(),mCamMoveUp(),mCamMoveDown(),
	mzFar(75000.0f),mzNear(1.0f),mYFOV(XM_PIDIV2), mCoords(0,0,0)
{
	// Initialize the view matrix
	mEye = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );
	mLookVector = XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f );
	mUp = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	mActualUp = XMVectorSet(0.0f,1.0f,0.0f,0.0f);
}

Camera::~Camera()
{
}

void Camera::setEye(DOUBLE x, DOUBLE y, DOUBLE z) {
	INT dX = (INT) floor(x);
	INT dY = (INT) floor(y);
	INT dZ = (INT) floor(z);
	mEye = XMVectorSet((FLOAT)(x - dX),
		(FLOAT)(y - dY),
		(FLOAT)(z - dZ),1.0f);
	mCoords = XMINT3(dX,dY,dZ);
}

void Camera::update(DXGI_SURFACE_DESC pSurfaceDesc)
{
	updateCameraMove();

	FLOAT mEyeX = XMVectorGetX(mEye);
	FLOAT mEyeY = XMVectorGetY(mEye);
	FLOAT mEyeZ = XMVectorGetZ(mEye);

	if (mEyeX > 1.0f) {
		INT xDiff = (INT)floor(mEyeX);
		mCoords.x+=xDiff;
		mEyeX-=xDiff;
	}
	else if (mEyeX < -1.0f) {
		INT xDiff = (INT)ceil(mEyeX);
		mCoords.x+=xDiff;
		mEyeX-=xDiff;
	}

	if (mEyeY > 1.0f) {
		INT yDiff = (INT)floor(mEyeY);
		mCoords.y+=yDiff;
		mEyeY-=yDiff;
	}
	else if (mEyeY < -1.0f) {
		INT yDiff = (INT)ceil(mEyeY);
		mCoords.y+=yDiff;
		mEyeY-=yDiff;
	}

	if (mEyeZ > 1.0f) {
		INT zDiff = (INT)floor(mEyeZ);
		mCoords.z+=zDiff;
		mEyeZ-=zDiff;
	}
	else if (mEyeZ < -1.0f) {
		INT zDiff = (INT)ceil(mEyeZ);
		mCoords.z+=zDiff;
		mEyeZ-=zDiff;
	}

	mEye = XMVectorSet(mEyeX,mEyeY,mEyeZ,0.0f);

	XMVECTOR lookAt = XMVectorAdd(mEye,mLookVector);

	mViewMatrix = XMMatrixLookAtLH( mEye, lookAt, mUp );

	FLOAT aspect = pSurfaceDesc.Width / (FLOAT)pSurfaceDesc.Height;

	mProjectionMatrix = XMMatrixPerspectiveFovLH( mYFOV, aspect, mzNear, mzFar);

	mViewProjectionMatrix = XMMatrixMultiply(mViewMatrix,mProjectionMatrix);

	XMVECTOR rightVec = XMVector3Normalize(XMVector3Cross(mLookVector,mActualUp));

	FLOAT height = 2 * tan(mYFOV/2);

	FLOAT nearH = height * mzNear;
	FLOAT nearW = nearH * aspect;

	FLOAT farH = height * mzFar;
	FLOAT farW = farH * aspect;

	FLOAT halfNearH = nearH/2;
	FLOAT halfNearW = nearW/2;

	FLOAT halfFarH = farH/2;
	FLOAT halfFarW = farW/2;

	XMVECTOR nearUpVec = mActualUp * halfNearH;
	XMVECTOR nearRightVec = rightVec * halfNearW;
	
	XMVECTOR farUpVec = mActualUp * halfFarH;
	XMVECTOR farRightVec = rightVec * halfFarW;

	XMVECTOR npc = mEye + (mLookVector * mzNear);

	XMVECTOR nptl = npc + nearUpVec - nearRightVec;

	XMVECTOR npbr = npc - nearUpVec - nearRightVec;

	XMVECTOR fpc = mEye + (mLookVector * mzFar);

	XMVECTOR fptl = fpc + farUpVec - farRightVec;

	XMVECTOR fpbr = fpc - farUpVec + farRightVec;

	mNearNormal = mLookVector;
	mFarNormal = -mLookVector;

	XMVECTOR ltv = nptl - fptl;
	XMVECTOR rbv = fpbr - npbr;

	mLeftNormal = XMVector3Normalize(XMVector3Cross(mActualUp,ltv));

	mRightNormal = XMVector3Normalize(XMVector3Cross(mActualUp,rbv));

	mTopNormal = XMVector3Normalize(XMVector3Cross(rightVec,ltv));

	mBottomNormal = XMVector3Normalize(XMVector3Cross(rightVec,rbv));

	mLeftTopNearPoint = nptl;
	mRightBottomFarPoint = fpbr;

}

LRESULT Camera::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		{
			if (mForceMouseLooking == FALSE) {
				mHeldMouseLooking = TRUE;
				MouseLookEnabled(hWnd);
				*pbNoFurtherProcessing = TRUE;
			}
			break;
		}
	case WM_LBUTTONUP:
		{
			if (mHeldMouseLooking == TRUE) {
				mHeldMouseLooking = FALSE;
				MouseLookDisabled();
				*pbNoFurtherProcessing = TRUE;
			}
			break;
		}
	case WM_MOUSEMOVE:
		{
			if (mMouseCentred == TRUE) {
				mMouseCentred = FALSE;
				*pbNoFurtherProcessing = TRUE;
			}
			else if (mHeldMouseLooking == TRUE || mForceMouseLooking == TRUE) {
				POINT mousePos;
				GetCursorPos( &mousePos );

				XMINT2 mouseDelta = XMINT2(mScreenCentreX - mousePos.x, mousePos.y - mScreenCentreY);

				updateCameraLook(mouseDelta);
				SetCursorPos(mScreenCentreX,mScreenCentreY);
				mMouseCentred = TRUE;
				*pbNoFurtherProcessing = TRUE;
			}
			break;
		}
	case WM_CAPTURECHANGED:
		{
			mHeldMouseLooking = FALSE;
			break;
		}
	case WM_KEYDOWN:
	case WM_KEYUP:
		{
			keyInteracted(hWnd, uMsg, wParam, pbNoFurtherProcessing);
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

	DOUBLE lookZXMag = sqrt(lookZ * lookZ + lookX * lookX);

	DOUBLE lrlookX = lookX/lookZXMag;
	DOUBLE lrlookZ = lookZ/lookZXMag;

	DOUBLE dX = deltaForward * lookX + deltaLeft * -lrlookZ;
	DOUBLE dY = deltaForward * lookY + deltaUp;
	DOUBLE dZ = deltaForward * lookZ + deltaLeft * lrlookX;

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
		mLookVector = XMVector3Normalize(XMVectorSet(
			(FLOAT) (sinX * cosY),
			(FLOAT) (-sinY),
			(FLOAT) (-cosX * cosY), 0.0f));

		mActualUp = XMVector3Normalize(XMVectorSet(
			(FLOAT) (sinX * sinY),
			(FLOAT) (cosY),
			(FLOAT) (-sinY * cosX), 0.0f));
}

void Camera::keyInteracted(HWND hWnd, UINT uMsg, WPARAM wParam, bool* pbNoFurtherProcessing)
{
	CameraButton* button = NULL;
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
	case ' ':
		{
			button = &mCamMoveUp;
			break;
		}
	case VK_CONTROL:
		{
			button = &mCamMoveDown;
			break;
		}
	case 'F':
		{
			if (uMsg == WM_KEYDOWN) {

				if (mForceMouseLooking == FALSE) {
					if (mHeldMouseLooking) {
						mHeldMouseLooking = FALSE;
						MouseLookDisabled();
					}
					MouseLookEnabled(hWnd);
				}
				else {
					MouseLookDisabled();
				}
				mForceMouseLooking = !mForceMouseLooking;
			}
			return;
		}
	default:
		{
			return;
		}
	}

	if (button != NULL) {
		if (uMsg == WM_KEYDOWN) {
			button->Push();
		}
		else {
			button->Release();
		}
	}

	*pbNoFurtherProcessing = TRUE;

}

void Camera::updateWindowDimensions()
{
	MONITORINFO mi;
	mi.cbSize = sizeof( MONITORINFO );
	DXUTGetMonitorInfo( DXUTMonitorFromWindow( DXUTGetHWND(), MONITOR_DEFAULTTONEAREST ), &mi );
	mScreenCentreX = ( mi.rcMonitor.left + mi.rcMonitor.right ) / 2;
	mScreenCentreY = ( mi.rcMonitor.top + mi.rcMonitor.bottom ) / 2;
}

void Camera::MouseLookEnabled(HWND hWnd)
{
	GetCursorPos( &mMouseStart );
	mMouseCentred = TRUE;

	SetCursorPos(mScreenCentreX,mScreenCentreY);
	SetCapture( hWnd );
	ShowCursor(FALSE);}

void Camera::MouseLookDisabled()
{
	mMouseCentred = FALSE;
	ReleaseCapture();
	SetCursorPos(mMouseStart.x,mMouseStart.y);
	ShowCursor(TRUE);}

CameraButton::CameraButton() : mPushed(FALSE),mDownTime(0),mUpTime(0),mLastProcessed(0) {
}

void CameraButton::Push()
{
	if (mPushed == TRUE) {
		return;
	}

	double currentTick = getTime();

	if (mLastProcessed >= mUpTime) {
		mDownTime = currentTick;
	}
	//If unprocessed since last key release event, it pretends
	//that the button was pushed down earlier
	else {
		double unprocessedTime = mUpTime - mLastProcessed;
		mDownTime = currentTick - unprocessedTime;
	}

	mPushed = TRUE;
}

void CameraButton::Release()
{
	mUpTime = getTime();
	mPushed = FALSE;
}

double CameraButton::GetTicksPressedFor()
{
	double curTime = getTime();
	double retTime;

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

//Returns TRUE if it passes
BOOL Camera::testFrustum( XMFLOAT3 pPos, XMINT3 pCoords, FLOAT pSphereRadius ) const
{
	XMVECTOR pos = XMVectorSet(pPos.x + (pCoords.x - mCoords.x),
		pPos.y + (pCoords.y - mCoords.y),
		pPos.z + (pCoords.z - mCoords.z),0.0f);

	XMVECTOR ltn = pos - mLeftTopNearPoint;

	FLOAT test1 = XMVectorGetX(XMVector3Dot(ltn,mNearNormal));
	if (test1 < 0) {
		if (test1 + pSphereRadius < 0) {
			return FALSE;
		}
	}

	FLOAT test2 = XMVectorGetX(XMVector3Dot(ltn,mLeftNormal));
	if (test2 < 0) {
		if (test2 + pSphereRadius < 0) {
			return FALSE;
		}
	}

	FLOAT test3 = XMVectorGetX(XMVector3Dot(ltn,mTopNormal));
	if (test3 < 0) {
		if (test3 + pSphereRadius < 0) {
			return FALSE;
		}
	}

	XMVECTOR rbf = pos - mRightBottomFarPoint;

	FLOAT test4 = XMVectorGetX(XMVector3Dot(rbf,mRightNormal));
	if (test4 < 0) {
		if (test4 + pSphereRadius < 0) {
			return FALSE;
		}
	}

	FLOAT test5 = XMVectorGetX(XMVector3Dot(rbf,mBottomNormal));
	if (test5 < 0) {
		if (test5 + pSphereRadius < 0) {
			return FALSE;
		}
	}

	FLOAT test6 = XMVectorGetX(XMVector3Dot(rbf,mFarNormal));
	if (test6 < 0) {
		if (test6 + pSphereRadius < 0) {
			return FALSE;
		}
	}

	return TRUE;
}