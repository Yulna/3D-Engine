#pragma once
#include "Geometry/AABB.h"
#include <list>

#define QUADTREE_MAX_ITEMS 2
#define QUADTREE_MIN_SIZE 10.0f

class GameObject;

class QuadtreeNode
{
public:
	QuadtreeNode(const AABB& box);
	virtual ~QuadtreeNode();

	bool isLeaf() const;

	void Insert(GameObject* obj);
	void Remove(GameObject* obj);

	void DebugDraw();

	void CreateChilds();
	void RedistributeChilds();


public:
	AABB box;
	std::list<GameObject*> objects;
	QuadtreeNode* parent = nullptr;
	QuadtreeNode* childs[4];

};


class Quadtree
{
public:
	Quadtree();
	virtual ~Quadtree();

	void Boundaries(AABB limits);
	void Clear();
	void Bake(const std::vector<GameObject*>& objects);
	void Insert(GameObject* obj);
	void Remove(GameObject* obj);

	void DebugDraw();

	template<typename TYPE>
	void CollectIntersections(std::vector<GameObject*>& objects, const TYPE& primitive) const;

public:
	QuadtreeNode* root_node = nullptr;
};

template<typename TYPE>
inline void Quadtree::CollectIntersections(std::vector<GameObject*>& objects, const TYPE& primitive) const
{
	if (root_node != nullptr)
	{
		root_node->CollectIntersections(objects, primitive);
	}
}