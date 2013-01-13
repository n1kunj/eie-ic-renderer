#pragma once
#ifndef CAMERA_H
#define CAMERA_H
#include <xnamath.h>

class Camera {
public:
	XMMATRIX mViewMatrix;
	XMMATRIX mProjectionMatrix;
	XMMATRIX mViewProjectionMatrix;

	void updateMatrices(DXGI_SURFACE_DESC pSurfaceDesc);
};

#endif