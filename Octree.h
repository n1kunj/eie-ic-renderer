#pragma once
#ifndef OCTREE_H
#define OCTREE_H
#include <unordered_map>

class Drawable;
class Camera;

class Octree {
public:
	Octree();
	~Octree();
	void add(Drawable* pDrawable);
	void remove(Drawable* pDrawable);
private:
	
};



#endif