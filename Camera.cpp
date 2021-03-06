#include "DXUT.h"
#include "Camera.h"
#include <windowsx.h>
#include <WinUser.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <sstream>
#include "Renderer.h"

#define CAMERALOOKSCALE 0.005

#define CAMERAMOVESPEEDSCALE (1.0/1000.0)

using namespace DirectX;

double getTime() {
	return 1000 * DXUTGetTime();
}

Camera::Camera(RendererSettings* pRendererSettings) : mHeldMouseLooking(FALSE),mMouseCentred(FALSE),
	mForceMouseLooking(FALSE), mMouseStart(), mLookDistanceX(0),
	mLookDistanceY(0),mCamMoveBackward(),mCamMoveForward(),
	mCamStrafeLeft(),mCamStrafeRight(),mCamMoveUp(),mCamMoveDown(),
	mCoords(0,0,0), mRendererSettings(pRendererSettings), mYFOV(XM_PIDIV2 * (2.0f/3.0f)),mSmoothLook(FALSE), mSmoothLookDistanceX(0), mSmoothLookDistanceY(0), mConstLook(FALSE), mSmoothMove(FALSE),mConstMove(FALSE)
{
	// Initialize the view matrix
	mEye = XMVectorSet( 0.0f, 5000.0f, 0.0f, 0.0f );
	mLookVector = XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f );
	mUp = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	mActualUp = XMVectorSet(0.0f,1.0f,0.0f,0.0f);
	mLastUpdateTime = getTime();
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
	mConstMove = FALSE;
	mSmoothMove = FALSE;
}

void Camera::update(DXGI_SURFACE_DESC pSurfaceDesc)
{
	DOUBLE updateTime = getTime();
	DOUBLE deltaTime = (updateTime - mLastUpdateTime)/1000.0;
	mLastUpdateTime = updateTime;

	if (mSmoothLook) {
		DOUBLE dx = mSmoothLookDistanceX - mLookDistanceX;
		DOUBLE dy = mSmoothLookDistanceY - mLookDistanceY;

		mLookDistanceX += mRotSpeed*deltaTime*dx;
		mLookDistanceY += mRotSpeed*deltaTime*dy;
		updateCameraLook(DirectX::XMINT2(0,0));
	}
	if (mConstLook) {
		mLookDistanceX +=mConstLookDX*deltaTime;
		mLookDistanceY +=mConstLookDY*deltaTime;
		updateCameraLook(DirectX::XMINT2(0,0));
	}

	mCameraMoveSpeed = mRendererSettings->cameraspeed * CAMERAMOVESPEEDSCALE;
	mzFar = mRendererSettings->zfar;
	mzNear = mRendererSettings->znear;
	updateCameraMove();

	if (mSmoothMove || mConstMove) {
		DOUBLE x = getEyeX();
		DOUBLE y = getEyeY();
		DOUBLE z = getEyeZ();

		DOUBLE dx = mSmoothMoveX - x;
		DOUBLE dy = mSmoothMoveY - y;
		DOUBLE dz = mSmoothMoveZ - z;

		DOUBLE newx;
		DOUBLE newy;
		DOUBLE newz;

		if (mSmoothMove) {
			newx = x + dx*mSmoothMoveSpeed*deltaTime;
			newy = y + dy*mSmoothMoveSpeed*deltaTime;
			newz = z + dz*mSmoothMoveSpeed*deltaTime;
		}
		else {
			newx = x + mConstMoveX*deltaTime;
			newy = y + mConstMoveY*deltaTime;
			newz = z + mConstMoveZ*deltaTime;
		}

		mEye = XMVectorSet((FLOAT)newx,(FLOAT)newy,(FLOAT)newz,1.0f);
		mCoords = XMINT3(0,0,0);
	}

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

	mProjectionMatrix = XMMatrixPerspectiveFovLH(mYFOV, aspect, mzNear, mzFar);

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

	mNearPlane = mLookVector;
	mNearPlane = XMVectorSetW(mNearPlane,fabs(XMVectorGetX(XMVector3Dot(npbr,mNearPlane))));

	mFarPlane = -mLookVector;
	mFarPlane = XMVectorSetW(mFarPlane,fabs(XMVectorGetX(XMVector3Dot(fptl,mFarPlane))));

	XMVECTOR ltv = nptl - fptl;
	XMVECTOR rbv = fpbr - npbr;

	mLeftPlane = XMVector3Normalize(XMVector3Cross(mActualUp,ltv));
	mLeftPlane = XMVectorSetW(mLeftPlane,fabs(XMVectorGetX(XMVector3Dot(fptl,mLeftPlane))));

	mRightPlane = XMVector3Normalize(XMVector3Cross(mActualUp,rbv));
	mRightPlane = XMVectorSetW(mRightPlane,fabs(XMVectorGetX(XMVector3Dot(npbr,mRightPlane))));

	mTopPlane = XMVector3Normalize(XMVector3Cross(rightVec,ltv));
	mTopPlane = XMVectorSetW(mTopPlane,fabs(XMVectorGetX(XMVector3Dot(fptl,mTopPlane))));

	mBottomPlane = XMVector3Normalize(XMVector3Cross(rightVec,rbv));
	mBottomPlane = XMVectorSetW(mBottomPlane,fabs(XMVectorGetX(XMVector3Dot(npbr,mBottomPlane))));

	mLeftTopNearPoint = nptl;
	mRightBottomFarPoint = fpbr;

	mNearPlaneAbs = XMVectorAbs(mNearPlane);
	mFarPlaneAbs = XMVectorAbs(mFarPlane);
	mLeftPlaneAbs = XMVectorAbs(mLeftPlane);
	mRightPlaneAbs = XMVectorAbs(mRightPlane);
	mTopPlaneAbs = XMVectorAbs(mTopPlane);
	mBottomPlaneAbs = XMVectorAbs(mBottomPlane);

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
				mSmoothLook = FALSE;
				mConstLook = FALSE;
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
	DOUBLE forwardDist = (DOUBLE) mCamMoveForward.GetTicksPressedFor() * mCameraMoveSpeed;
	DOUBLE backwardDist = (DOUBLE) mCamMoveBackward.GetTicksPressedFor() * mCameraMoveSpeed;
	DOUBLE leftDist = (DOUBLE) mCamStrafeLeft.GetTicksPressedFor() * mCameraMoveSpeed;
	DOUBLE rightDist = (DOUBLE) mCamStrafeRight.GetTicksPressedFor() * mCameraMoveSpeed;
	DOUBLE upDist = (DOUBLE) mCamMoveUp.GetTicksPressedFor() * mCameraMoveSpeed;
	DOUBLE downDist = (DOUBLE) mCamMoveDown.GetTicksPressedFor() * mCameraMoveSpeed;
	
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

		mLookDistanceX += pMoveDelta.x;
		mLookDistanceY += pMoveDelta.y;

		if (mLookDistanceX > 2 * M_PI / CAMERALOOKSCALE) {
			mLookDistanceX-= 2 * M_PI / CAMERALOOKSCALE;
		}
		else if (mLookDistanceX < -2 * M_PI / CAMERALOOKSCALE) {
			mLookDistanceX+= 2 * M_PI / CAMERALOOKSCALE;
		}

		//Set X axis vars
		DOUBLE curX = CAMERALOOKSCALE * mLookDistanceX;
		DOUBLE sinX = sin(curX);
		DOUBLE cosX = cos(curX);

		//Set Y axis vars
		DOUBLE curY = CAMERALOOKSCALE * mLookDistanceY;
		DOUBLE sinY;
		DOUBLE cosY;

		//Nik's super awesome FPS style camera. Won't let you look upside down.
		if (curY >= M_PI_2)
		{
			mLookDistanceY = M_PI_2 / CAMERALOOKSCALE;

			sinY = 1;
			cosY = 0.0001f;
		}
		else if (curY <= -M_PI_2)
		{
			mLookDistanceY = -M_PI_2 / CAMERALOOKSCALE;
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
		mSmoothMove = FALSE;
		mConstMove = FALSE;
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

	FLOAT test1 = XMVectorGetX(XMVector3Dot(ltn,mNearPlane));
	if (test1 < 0) {
		if (test1 + pSphereRadius < 0) {
			return FALSE;
		}
	}

	FLOAT test2 = XMVectorGetX(XMVector3Dot(ltn,mLeftPlane));
	if (test2 < 0) {
		if (test2 + pSphereRadius < 0) {
			return FALSE;
		}
	}

	FLOAT test3 = XMVectorGetX(XMVector3Dot(ltn,mTopPlane));
	if (test3 < 0) {
		if (test3 + pSphereRadius < 0) {
			return FALSE;
		}
	}

	XMVECTOR rbf = pos - mRightBottomFarPoint;

	FLOAT test4 = XMVectorGetX(XMVector3Dot(rbf,mRightPlane));
	if (test4 < 0) {
		if (test4 + pSphereRadius < 0) {
			return FALSE;
		}
	}

	FLOAT test5 = XMVectorGetX(XMVector3Dot(rbf,mBottomPlane));
	if (test5 < 0) {
		if (test5 + pSphereRadius < 0) {
			return FALSE;
		}
	}

	FLOAT test6 = XMVectorGetX(XMVector3Dot(rbf,mFarPlane));
	if (test6 < 0) {
		if (test6 + pSphereRadius < 0) {
			return FALSE;
		}
	}

	return TRUE;
}

BOOL Camera::testFrustumAABB( DirectX::XMFLOAT3 pPos, DirectX::XMINT3 pCoords, DirectX::XMFLOAT3 pLenXYZ ) const
{
	XMVECTOR pos = XMVectorSet(pPos.x + (pCoords.x - mCoords.x),
		pPos.y + (pCoords.y - mCoords.y),
		pPos.z + (pCoords.z - mCoords.z),0.0f);

	XMVECTOR size = XMLoadFloat3(&pLenXYZ);

	XMVECTOR planes[6] = {mNearPlane,mFarPlane,mLeftPlane,mRightPlane,mTopPlane,mBottomPlane};

	XMVECTOR planeAbss[6] = {mNearPlaneAbs, mFarPlaneAbs,mLeftPlaneAbs,mRightPlaneAbs,mTopPlaneAbs,mBottomPlaneAbs};

	for (int i = 0; i < 6; i++) {
		XMVECTOR &plane = planes[i];
		XMVECTOR &planeAbs = planeAbss[i];

		float d = XMVectorGetX(XMVector3Dot(pos,plane));

		float r = XMVectorGetX(XMVector3Dot(size,planeAbs));

		float d_p_r = d + r + XMVectorGetW(plane);

		if (d_p_r < 0) {
			return FALSE;
		}

	}
	return TRUE;
}

void Camera::setLook( DOUBLE zenith, DOUBLE azimuth )
{
	mSmoothLook = FALSE;
	mConstLook = FALSE;
	//azi = rotation
	//zeni = inclination
	mLookDistanceX = -(azimuth/180.0) * (2 * M_PI / CAMERALOOKSCALE);
	mLookDistanceY = -(zenith/90) * (M_PI_2/CAMERALOOKSCALE);
	updateCameraLook(DirectX::XMINT2(0,0));
}

void Camera::smoothLook( DOUBLE zenith, DOUBLE azimuth, DOUBLE rotSpeed)
{
	mSmoothLook = TRUE;
	mConstLook = FALSE;
	mSmoothLookDistanceX = -(azimuth/180.0) * (2 * M_PI / CAMERALOOKSCALE);
	mSmoothLookDistanceY = -(zenith/90) * (M_PI_2/CAMERALOOKSCALE);

	if (mSmoothLookDistanceX > 2 * M_PI / CAMERALOOKSCALE) {
		mSmoothLookDistanceX-= 2 * M_PI / CAMERALOOKSCALE;
	}
	else if (mSmoothLookDistanceX < -2 * M_PI / CAMERALOOKSCALE) {
		mSmoothLookDistanceX+= 2 * M_PI / CAMERALOOKSCALE;
	}

	mRotSpeed = rotSpeed;
}

void Camera::constLook( DOUBLE dz, DOUBLE da )
{
	mSmoothLook = FALSE;
	mConstLook = TRUE;
	mConstLookDX = -(da/180.0) * (2 * M_PI / CAMERALOOKSCALE);
	mConstLookDY = -(dz/90) * (M_PI_2/CAMERALOOKSCALE);
}

void Camera::smoothMove( DOUBLE x, DOUBLE y, DOUBLE z, DOUBLE speed)
{
	mSmoothMove = TRUE;
	mConstMove = FALSE;
	mSmoothMoveSpeed = speed;
	mSmoothMoveX = x;
	mSmoothMoveY = y;
	mSmoothMoveZ = z;
}

void Camera::constMove( DOUBLE speedX, DOUBLE speedY, DOUBLE speedZ )
{
	mSmoothMove = FALSE;
	mConstMove = TRUE;
	mConstMoveX = speedX;
	mConstMoveY = speedY;
	mConstMoveZ = speedZ;
}
