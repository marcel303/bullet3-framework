#pragma once

#include "LinearMath/btAlignedObjectArray.h"

class btCollisionShape;

struct ShapeVertex
{
	float xyzw[4];
	float normal[3];
	float uv[2];
};

void computeSoftBodyVertices(
	const btCollisionShape * collisionShape,
	btAlignedObjectArray<ShapeVertex> & out_vertices,
	btAlignedObjectArray<int> & out_indices);

void generateShapeMesh(
	btCollisionShape * collisionShape,
	btAlignedObjectArray<ShapeVertex> & out_vertices,
	btAlignedObjectArray<int> & out_indices);
