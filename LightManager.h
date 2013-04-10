#include "DXUT.h"
#pragma once
#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H
#include <vector>
#include <deque>
#include "Camera.h"
#include "Texture2D.h"

struct PointLight {
	DOUBLE x;
	DOUBLE y;
	DOUBLE z;
	FLOAT attenuationEnd;
	DirectX::XMFLOAT3 colour;
	FLOAT ambient;
};

struct PointLightGPU {
	DirectX::XMFLOAT3 viewPos;
	FLOAT attenuationEnd;
	DirectX::XMFLOAT3 colour;
	FLOAT ambient;
};

class LightManager {
private:
	std::vector<PointLight*> lightVector;
	std::deque<size_t> freeSlots;
public:

	LightManager() {
		lightVector.reserve(100000);
	}

	//Returns the index of the light in the manager
	size_t addLight(PointLight* pLight) {
		if (freeSlots.size() == 0) {
			lightVector.push_back(pLight);
			return lightVector.size()-1;
		}
		else {
			size_t slot = freeSlots.front();
			freeSlots.pop_front();
			lightVector[slot] = pLight;
			return slot;
		}
	}

	//Doesnt range check, so don't be stupid
	void removeLight(size_t lightIndex) {
		lightVector[lightIndex] = NULL;
		freeSlots.push_back(lightIndex);
	}

	//Returns the number of lights loaded into the buffer
	UINT updateLightBuffer(ID3D11DeviceContext* pd3dContext, const Camera* pCamera,StructuredBuffer<PointLightGPU>* pLightListSB) {

		using namespace DirectX;

		std::vector<std::pair<FLOAT,PointLightGPU> > tempList;
		tempList.reserve(lightVector.size());

		DirectX::XMINT3 offset = pCamera->mCoords;
		DirectX::XMFLOAT3 eye;
		XMStoreFloat3(&eye,pCamera->mEye);

		for (int i = 0; i < lightVector.size(); i++) {
			const PointLight* pl = lightVector[i];
			if (pl != NULL) {
				PointLightGPU plgpu;
				plgpu.ambient = pl->ambient;
				plgpu.attenuationEnd = pl->attenuationEnd;
				plgpu.colour = pl->colour;

				FLOAT posx = (FLOAT)(pl->x - offset.x);
				FLOAT posy = (FLOAT)(pl->y - offset.y);
				FLOAT posz = (FLOAT)(pl->z - offset.z);
				plgpu.viewPos = DirectX::XMFLOAT3(posx,posy,posz);

				posx -= eye.x;
				posy -= eye.y;
				posz -= eye.z;
				FLOAT distsq = posx*posx+posy*posy+posz*posz;
				tempList.push_back(std::pair<FLOAT,PointLightGPU>(distsq,plgpu));
			}
		}

		//Sort by camera distance
		std::sort(tempList.begin(),tempList.end(),LightManager::lengthCompare);

		PointLightGPU* llist = pLightListSB->MapDiscard(pd3dContext);
		UINT numLights = 0;
		UINT maxLights = pLightListSB->mElements;

		//Frustum test lights and put the untransformed lights into the structured buffer
		for (int i = 0; i < tempList.size(); i++) {
			if (numLights == maxLights) {
				break;
			}

			PointLightGPU* plgpu = &tempList[i].second;
			BOOL passed = pCamera->testFrustum(plgpu->viewPos,offset,plgpu->attenuationEnd);
			if (passed) {
				llist[numLights] = *plgpu;
				numLights++;
			}
		}

		XMVector3TransformCoordStream(&llist->viewPos,sizeof(PointLightGPU),&llist->viewPos,sizeof(PointLightGPU),numLights,pCamera->mViewMatrix);

		pLightListSB->Unmap(pd3dContext);

		return numLights;
	}

private:

	static BOOL lengthCompare(std::pair<FLOAT,PointLightGPU> a, std::pair<FLOAT,PointLightGPU> b) {
		return a.first < b.first;
	}
};

#endif