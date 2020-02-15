#include "ShapeGeneration.h"
#include "OpenGLWindow/ShapeData.h"

#include "btBulletDynamicsCommon.h"

#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h" // to create a tesselation of a generic btConvexShape
#include "BulletCollision/CollisionShapes/btConvexPolyhedron.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"
#include "LinearMath/btTransform.h"

static void appendPrim(
	const ShapeVertex * __restrict vertices,
	const int numVertices,
	const int * __restrict indices,
	const int numIndices,
	const btTransform * transform,
	const btVector3 & halfExtents,
	btAlignedObjectArray<ShapeVertex> & out_vertices,
	btAlignedObjectArray<int> & out_indices)
{
	const int baseVertex = out_vertices.size();
	
	out_vertices.resize(out_vertices.size() + numVertices);
	for (int i = 0; i < numVertices; ++i)
		out_vertices[baseVertex + i] = vertices[i];

	for (int i = 0; i < numVertices; ++i)
	{
		float & x = out_vertices[baseVertex + i].xyzw[0];
		float & y = out_vertices[baseVertex + i].xyzw[1];
		float & z = out_vertices[baseVertex + i].xyzw[2];
		
		x *= halfExtents.x();
		y *= halfExtents.y();
		z *= halfExtents.z();
		
		if (transform != nullptr)
		{
			const btVector3 p = (*transform) * btVector3(x, y, z);
			
			x = p.x();
			y = p.y();
			z = p.z();
		}
	}
	
	const int baseIndex = out_indices.size();
	out_indices.resize(out_indices.size() + numIndices);
	for (int i = 0; i < numIndices; ++i)
		out_indices[baseIndex + i] = baseVertex + indices[i];
}

class MyTriangleCollector2 : public btTriangleCallback, public btInternalTriangleIndexCallback
{
public:
	btVector3 m_aabbMin, m_aabbMax;
	btScalar m_textureScaling = 1.;
	
	btAlignedObjectArray<ShapeVertex> m_vertices;
	btAlignedObjectArray<int> m_indices;

	MyTriangleCollector2(
		const btVector3& aabbMin,
		const btVector3& aabbMax)
		: m_aabbMin(aabbMin)
		, m_aabbMax(aabbMax)
	{
		m_vertices.reserve(1 << 16);
		m_indices.reserve(1 << 16);
	}

	virtual void processTriangle(btVector3 * tris, int partId, int triangleIndex) override final
	{
		const int baseVertex = m_vertices.size();
		const int baseIndex = m_indices.size();
		
		m_vertices.resize(m_vertices.size() + 3);
		m_indices.resize(m_indices.size() + 3);
		
		m_indices[baseIndex + 0] = baseVertex + 0;
		m_indices[baseIndex + 1] = baseVertex + 1;
		m_indices[baseIndex + 2] = baseVertex + 2;
		
		const btVector3 extents = m_aabbMax - m_aabbMin;
		
		for (int k = 0; k < 3; k++)
		{
			const btVector3 normal = (tris[0] - tris[1]).cross(tris[0] - tris[2]).safeNormalize();
			
			auto & v = m_vertices[baseVertex + k];
			
			for (int l = 0; l < 3; l++)
			{
				v.xyzw[l] = tris[k][l];
				v.normal[l] = normal[l];
			}
			
			v.uv[0] = (1. - (v.xyzw[0] - m_aabbMin[0]) / extents[0]) * m_textureScaling;
			v.uv[1] = (1. - (v.xyzw[1] - m_aabbMin[1]) / extents[1]) * m_textureScaling;
		}
	}
	
	virtual void internalProcessTriangleIndex(btVector3 * triangle, int partId, int triangleIndex) override final
	{
		processTriangle(triangle, partId, triangleIndex);
	}
};

void computeSoftBodyVertices(
	const btCollisionShape * collisionShape,
	btAlignedObjectArray<ShapeVertex> & out_vertices,
	btAlignedObjectArray<int> & out_indices)
{
	btAssert(collisionShape->getUserPointer());
	if (collisionShape->getUserPointer() == nullptr)
		return;
	
	const btSoftBody * psb = (btSoftBody*)collisionShape->getUserPointer();
	out_vertices.resize(psb->m_faces.size() * 3);

	for (int i = 0; i < psb->m_faces.size(); ++i) // Foreach face
	{
		for (int k = 0; k < 3; ++k) // Foreach vertex on a face
		{
			const int currentIndex = i * 3 + k;
			
			for (int j = 0; j < 3; j++)
				out_vertices[currentIndex].xyzw[j] = psb->m_faces[i].m_n[k]->m_x[j];
			out_vertices[currentIndex].xyzw[3] = 1.;
			
			for (int j = 0; j < 3; ++j)
				out_vertices[currentIndex].normal[j] = psb->m_faces[i].m_n[k]->m_n[j];
			for (int j = 0; j < 2; ++j)
				out_vertices[currentIndex].uv[j] = 0.5; // we don't have UV info...
			
			out_indices.push_back(currentIndex);
		}
	}
}

static void appendShape(
	const btCollisionShape * collisionShape,
	const btTransform * shapeTransform,
	btAlignedObjectArray<ShapeVertex> & out_vertices,
	btAlignedObjectArray<int> & out_indices)
{
	const int vertexStrideInBytes = sizeof(ShapeVertex);
	btAssert(vertexStrideInBytes == 9 * sizeof(float));
	
	if (collisionShape->getShapeType() == BOX_SHAPE_PROXYTYPE)
	{
		const btBoxShape * boxShape = (btBoxShape*)collisionShape;
		const btVector3 halfExtents = boxShape->getHalfExtentsWithMargin();
		
		appendPrim(
			(ShapeVertex*)cube_vertices_textured,
			sizeof(cube_vertices_textured) / vertexStrideInBytes,
			cube_indices,
			sizeof(cube_indices) / sizeof(cube_indices[0]),
			shapeTransform,
			halfExtents,
			out_vertices,
			out_indices);
	}
	else if (collisionShape->getShapeType() == SPHERE_SHAPE_PROXYTYPE)
	{
		const btSphereShape * sphereShape = (btSphereShape*)collisionShape;
		const btScalar sphereSize = 2. * sphereShape->getRadius();

		appendPrim(
			(ShapeVertex*)textured_detailed_sphere_vertices,
			sizeof(textured_detailed_sphere_vertices) / vertexStrideInBytes,
			textured_detailed_sphere_indices,
			sizeof(textured_detailed_sphere_indices) / sizeof(textured_detailed_sphere_indices[0]),
			shapeTransform,
			btVector3(sphereSize, sphereSize, sphereSize),
			out_vertices,
			out_indices);
	}
	else if (collisionShape->getShapeType() == CAPSULE_SHAPE_PROXYTYPE)
	{
		const btCapsuleShape * capsuleShape = (btCapsuleShape*)collisionShape;
		const int up = capsuleShape->getUpAxis();
		const btScalar halfHeight = capsuleShape->getHalfHeight();

		const btScalar radius = capsuleShape->getRadius();
		const btScalar sphereSize = 2. * radius;

		const btVector3 radiusScale = btVector3(sphereSize, sphereSize, sphereSize);

		const int numVertices = sizeof(textured_detailed_sphere_vertices) / vertexStrideInBytes;
		const int numIndices = sizeof(textured_detailed_sphere_indices) / sizeof(textured_detailed_sphere_indices[0]);
		const int * indices = textured_detailed_sphere_indices;
		
		btAlignedObjectArray<ShapeVertex> transformedVertices;
		transformedVertices.resize(numVertices);
		for (size_t i = 0; i < numVertices; ++i)
			transformedVertices[i] = ((ShapeVertex*)textured_detailed_sphere_vertices)[i];
		
		for (int i = 0; i < numVertices; ++i)
		{
			transformedVertices[i].xyzw[0] *= radiusScale.x();
			transformedVertices[i].xyzw[1] *= radiusScale.y();
			transformedVertices[i].xyzw[2] *= radiusScale.z();
			
			if (transformedVertices[i].xyzw[up] > 0)
				transformedVertices[i].xyzw[up] += halfHeight;
			else
				transformedVertices[i].xyzw[up] -= halfHeight;
		}
		
		appendPrim(&transformedVertices[0], numVertices, indices, numIndices, shapeTransform, btVector3(1., 1., 1.), out_vertices, out_indices);
	}
	else if (collisionShape->getShapeType() == MULTI_SPHERE_SHAPE_PROXYTYPE)
	{
		const btMultiSphereShape * multiSphereShape = (btMultiSphereShape*)collisionShape;
		
		for (int i = 0; i < multiSphereShape->getSphereCount(); ++i)
		{
			const btVector3 position = multiSphereShape->getSpherePosition(i);
			const btScalar radius = multiSphereShape->getSphereRadius(i);
			const btScalar sphereSize = 2. * radius;

			btTransform transform;
			transform.setIdentity();
			transform.setOrigin(position);
			
			if (shapeTransform != nullptr)
				transform = (*shapeTransform) * transform;
			
			appendPrim(
				(ShapeVertex*)textured_detailed_sphere_vertices,
				sizeof(textured_detailed_sphere_vertices) / vertexStrideInBytes,
				textured_detailed_sphere_indices,
				sizeof(textured_detailed_sphere_indices) / sizeof(textured_detailed_sphere_indices[0]),
				&transform,
				btVector3(sphereSize, sphereSize, sphereSize),
				out_vertices,
				out_indices);
		}
	}
	else if (collisionShape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
	{
		btCompoundShape * compound = (btCompoundShape*)collisionShape;
		for (int i = 0; i < compound->getNumChildShapes(); ++i)
			appendShape(compound->getChildShape(i), &compound->getChildTransform(i), out_vertices, out_indices);
	}
	else if (collisionShape->getShapeType() == STATIC_PLANE_PROXYTYPE)
	{
		const btStaticPlaneShape * staticPlaneShape = static_cast<const btStaticPlaneShape*>(collisionShape);
		const btScalar planeConst = staticPlaneShape->getPlaneConstant();
		const btVector3 & planeNormal = staticPlaneShape->getPlaneNormal();
		const btVector3 planeOrigin = planeNormal * planeConst;
		
		btVector3 vec0, vec1;
		btPlaneSpace1(planeNormal, vec0, vec1);

		const btScalar vecLen = 128;
		const btVector3 verts[4] =
		{
			planeOrigin + vec0 * vecLen + vec1 * vecLen,
			planeOrigin - vec0 * vecLen + vec1 * vecLen,
			planeOrigin - vec0 * vecLen - vec1 * vecLen,
			planeOrigin + vec0 * vecLen - vec1 * vecLen
		};

		const int indices[6] =
		{
			0, 1, 2, 0, 2, 3
		};

		ShapeVertex vertices[4];

		for (int i = 0; i < 4; i++)
		{
			const btVector3 & pos = verts[i];

			vertices[i].xyzw[0] = pos[0];
			vertices[i].xyzw[1] = pos[1];
			vertices[i].xyzw[2] = pos[2];
			vertices[i].xyzw[3] = 1.;
			vertices[i].normal[0] = planeNormal[0];
			vertices[i].normal[1] = planeNormal[1];
			vertices[i].normal[2] = planeNormal[2];
		}

		vertices[0].uv[0] = +vecLen / 2;
		vertices[0].uv[1] = +vecLen / 2;
		vertices[1].uv[0] = -vecLen / 2;
		vertices[1].uv[1] = +vecLen / 2;
		vertices[2].uv[0] = -vecLen / 2;
		vertices[2].uv[1] = -vecLen / 2;
		vertices[3].uv[0] = +vecLen / 2;
		vertices[3].uv[1] = -vecLen / 2;

		appendPrim(vertices, 4, indices, 6, shapeTransform, btVector3(1., 1., 1.), out_vertices, out_indices);
	}
	else if (collisionShape->getShapeType() == TERRAIN_SHAPE_PROXYTYPE)
	{
		const btHeightfieldTerrainShape * heightField = static_cast<const btHeightfieldTerrainShape*>(collisionShape);
		
		btVector3 aabbMin(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
		btVector3 aabbMax(+BT_LARGE_FLOAT, +BT_LARGE_FLOAT, +BT_LARGE_FLOAT);
		
		MyTriangleCollector2 triangleCollector(aabbMin, aabbMax);
		if (heightField->getUserValue3())
			triangleCollector.m_textureScaling = heightField->getUserValue3();
		
		heightField->processAllTriangles(&triangleCollector, aabbMin, aabbMax);
		
		// todo : textures for shapes
		//int userImage = heightField->getUserIndex2();
		//if (userImage == -1)
		//	userImage = m_data->m_checkedTexture;
		
		if (triangleCollector.m_vertices.size() > 0)
		{
			appendPrim(
				&triangleCollector.m_vertices[0],
				triangleCollector.m_vertices.size(),
				&triangleCollector.m_indices[0],
				triangleCollector.m_indices.size(),
				shapeTransform,
				btVector3(1., 1., 1.),
				out_vertices,
				out_indices);
		}
	}
	else if (collisionShape->getShapeType() == SOFTBODY_SHAPE_PROXYTYPE)
	{
		btAlignedObjectArray<ShapeVertex> vertices;
		btAlignedObjectArray<int> indices;
		computeSoftBodyVertices(collisionShape, vertices, indices);
		if (vertices.size() > 0)
			appendPrim(&vertices[0], vertices.size(), &indices[0], indices.size(), shapeTransform, btVector3(1., 1., 1.), out_vertices, out_indices);
	}
	else if (collisionShape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
	{
		const btTriangleMeshShape * triangleMesh = static_cast<const btTriangleMeshShape*>(collisionShape);

		btVector3 aabbMin(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
		btVector3 aabbMax(+BT_LARGE_FLOAT, +BT_LARGE_FLOAT, +BT_LARGE_FLOAT);
		
		MyTriangleCollector2 triangleCollector(aabbMin, aabbMax);
		
		triangleMesh->getMeshInterface()->InternalProcessAllTriangles(&triangleCollector, aabbMin, aabbMax);
		
		if (triangleCollector.m_vertices.size() > 0)
		{
			appendPrim(
				&triangleCollector.m_vertices[0],
				triangleCollector.m_vertices.size(),
				&triangleCollector.m_indices[0],
				triangleCollector.m_indices.size(),
				shapeTransform,
				btVector3(1., 1., 1.),
				out_vertices,
				out_indices);
		}
	}
	else if (collisionShape->getShapeType() == CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE)
	{
		const btConvexTriangleMeshShape * convexMesh = static_cast<const btConvexTriangleMeshShape*>(collisionShape);

		btVector3 aabbMin(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
		btVector3 aabbMax(+BT_LARGE_FLOAT, +BT_LARGE_FLOAT, +BT_LARGE_FLOAT);
		
		MyTriangleCollector2 triangleCollector(aabbMin, aabbMax);
		
		convexMesh->getMeshInterface()->InternalProcessAllTriangles(&triangleCollector, aabbMin, aabbMax);
		
		if (triangleCollector.m_vertices.size() > 0)
		{
			appendPrim(
				&triangleCollector.m_vertices[0],
				triangleCollector.m_vertices.size(),
				&triangleCollector.m_indices[0],
				triangleCollector.m_indices.size(),
				shapeTransform,
				btVector3(1., 1., 1.),
				out_vertices,
				out_indices);
		}
	}
	else if (collisionShape->isConvex())
	{
		const btConvexShape * convex = (btConvexShape*)collisionShape;
		
		const btConvexPolyhedron * pol = nullptr;
		if (convex->isPolyhedral())
		{
			const btPolyhedralConvexShape * poly = (btPolyhedralConvexShape*)convex;
			pol = poly->getConvexPolyhedron();
		}

		btAlignedObjectArray<ShapeVertex> vertices;
		btAlignedObjectArray<int> indices;
		
		if (pol != nullptr)
		{
			for (int v = 0; v < pol->m_vertices.size(); v++)
			{
				ShapeVertex vertex;
				vertex.xyzw[0] = pol->m_vertices[v][0];
				vertex.xyzw[1] = pol->m_vertices[v][1];
				vertex.xyzw[2] = pol->m_vertices[v][2];
				vertex.xyzw[3] = 1.;
				
				btVector3 norm = pol->m_vertices[v];
				norm.safeNormalize();
				vertex.normal[0] = norm[0];
				vertex.normal[1] = norm[1];
				vertex.normal[2] = norm[2];
				
				vertex.uv[0] = .5;
				vertex.uv[1] = .5;
				
				vertices.push_back(vertex);
			}
			
			for (int f = 0; f < pol->m_faces.size(); f++)
			{
				for (int ii = 2; ii < pol->m_faces[f].m_indices.size(); ii++)
				{
					indices.push_back(pol->m_faces[f].m_indices[0]);
					indices.push_back(pol->m_faces[f].m_indices[ii - 1]);
					indices.push_back(pol->m_faces[f].m_indices[ii]);
				}
			}
		}
		else
		{
			btShapeHull * hull = new btShapeHull(convex);
			hull->buildHull(0.0, 1);

			{
				for (int t = 0; t < hull->numTriangles(); ++t)
				{
					const int index0 = hull->getIndexPointer()[t * 3 + 0];
					const int index1 = hull->getIndexPointer()[t * 3 + 1];
					const int index2 = hull->getIndexPointer()[t * 3 + 2];
					
					const btVector3 & pos0 = hull->getVertexPointer()[index0];
					const btVector3 & pos1 = hull->getVertexPointer()[index1];
					const btVector3 & pos2 = hull->getVertexPointer()[index2];
					
					const btVector3 triNormal = (pos1 - pos0).cross(pos2 - pos0).safeNormalize();

					for (int v = 0; v < 3; v++)
					{
						const int index = hull->getIndexPointer()[t * 3 + v];
						const btVector3 & pos = hull->getVertexPointer()[index];
						
						ShapeVertex vertex;
						vertex.xyzw[0] = pos[0];
						vertex.xyzw[1] = pos[1];
						vertex.xyzw[2] = pos[2];
						vertex.xyzw[3] = 1.;
						vertex.normal[0] = triNormal[0];
						vertex.normal[1] = triNormal[1];
						vertex.normal[2] = triNormal[2];
						vertex.uv[0] = .5;
						vertex.uv[1] = .5;
						
						indices.push_back(vertices.size());
						vertices.push_back(vertex);
					}
				}
			}
			
			delete hull;
		}

		if (vertices.size() > 0)
		{
			appendPrim(
				&vertices[0],
				vertices.size(),
				&indices[0],
				indices.size(),
				shapeTransform,
				btVector3(1., 1., 1.),
				out_vertices,
				out_indices);
		}
	}
	else
	{
		//printf("unknown collision shape type: %d\n", collisionShape->getShapeType());
		
		/*
		TRIANGLE_SHAPE_PROXYTYPE
		TETRAHEDRAL_SHAPE_PROXYTYPE
		CONE_SHAPE_PROXYTYPE
		CYLINDER_SHAPE_PROXYTYPE
		*/
	}
}

void generateShapeMesh(
	btCollisionShape * collisionShape,
	btAlignedObjectArray<ShapeVertex> & out_vertices,
	btAlignedObjectArray<int> & out_indices)
{
	appendShape(collisionShape, nullptr, out_vertices, out_indices);
}
