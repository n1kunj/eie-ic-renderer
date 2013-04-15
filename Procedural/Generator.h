#include "DXUT.h"
#pragma once
#ifndef GENERATOR_H
#define GENERATOR_H
#include <deque>
#include <memory>

class DistantTextures {
public:
	DirectX::XMFLOAT3 mColour;

	DistantTextures() {
		mColour = DirectX::XMFLOAT3(1.0f,0.0f,0.0f);
	}
};

typedef std::shared_ptr<DistantTextures> DTPTR;

class Generator {
private:
	std::deque<DTPTR> mTextureQueue;
public:

	Generator() {

	}
	~Generator() {

	}

	void InitialiseDistantTile(DTPTR pDistantTexture) {
		mTextureQueue.push_back(pDistantTexture);
	}
	void Generate(UINT pMaxRuntimeMillis) {
		if (mTextureQueue.size() == 0) {
			return;
		}
		DTPTR first = mTextureQueue.front();
		mTextureQueue.pop_front();

		if (first.unique()) {
			first.reset();
		}
		else {
			first->mColour = DirectX::XMFLOAT3(0.0f,1.0f,0.0f);
		}
	}
};

#endif