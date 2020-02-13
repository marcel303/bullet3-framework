#include "FrameworkDebugDrawer.h"
#include "FrameworkGuiHelper.h"

#include "CommonInterfaces/CommonGraphicsAppInterface.h"
#include "CommonInterfaces/CommonRenderInterface.h"

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h" // to create a tesselation of a generic btConvexShape
#include "BulletCollision/CollisionShapes/btConvexPolyhedron.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"

#include "framework.h"

#include <functional>

FrameworkGUIHelperInterface::~FrameworkGUIHelperInterface()
{
}

void FrameworkGUIHelperInterface::createRigidBodyGraphicsObject(btRigidBody* body, const btVector3& color)
{
	createCollisionObjectGraphicsObject(body, color);
}

void FrameworkGUIHelperInterface::createCollisionObjectGraphicsObject(btCollisionObject* body, const btVector3& color)
{
	if (body->getUserIndex() >= 0)
		return;

	const btCollisionShape * shape = body->getCollisionShape();
	const btTransform startTransform = body->getWorldTransform();
	
	const int graphicsShapeId = shape->getUserIndex();
	
	if (graphicsShapeId >= 0)
	{
		const int graphicsInstanceId = m_renderInterface->registerGraphicsInstance(
			graphicsShapeId,
			startTransform.getOrigin(),
			startTransform.getRotation(),
			color,
			btVector3(1., 1., 1.));
		
		body->setUserIndex(graphicsInstanceId);
	}
}

#include "OpenGLWindow/ShapeData.h"

struct FrameworkVertex
{
	float xyzw[4];
	float normal[3];
	float uv[2];
};

static void appendPrim(
	const FrameworkVertex * __restrict vertices,
	const int numVertices,
	const int * __restrict indices,
	const int numIndices,
	const btTransform * transform,
	const btVector3 & halfExtents,
	btAlignedObjectArray<FrameworkVertex> & out_vertices,
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
	
	btAlignedObjectArray<FrameworkVertex> m_vertices;
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

static void computeSoftBodyVertices(
	const btCollisionShape * collisionShape,
	btAlignedObjectArray<FrameworkVertex> & out_vertices,
	btAlignedObjectArray<int> & out_indices)
{
	b3Assert(collisionShape->getUserPointer());
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

void FrameworkGUIHelperInterface::createCollisionShapeGraphicsObject(btCollisionShape* collisionShape)
{
	// already has a graphics object ?
	
	if (collisionShape->getUserIndex() >= 0)
		return;
	
	const int vertexStrideInBytes = 9 * sizeof(float);
	btAssert(sizeof(FrameworkVertex) == vertexStrideInBytes);

	btAlignedObjectArray<FrameworkVertex> out_vertices;
	btAlignedObjectArray<int> out_indices;
	
	std::function<void(btCollisionShape * collisionShape, const btTransform * shapeTransform)> appendShape;
	
	appendShape = [&out_vertices, &out_indices, &appendShape](btCollisionShape * collisionShape, const btTransform * shapeTransform)
	{
		if (collisionShape->getShapeType() == BOX_SHAPE_PROXYTYPE)
		{
			const btBoxShape * boxShape = (btBoxShape*)collisionShape;
			const btVector3 halfExtents = boxShape->getHalfExtentsWithMargin();
			
			appendPrim(
				(FrameworkVertex*)cube_vertices_textured,
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
				(FrameworkVertex*)textured_detailed_sphere_vertices,
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
			
			btAlignedObjectArray<FrameworkVertex> transformedVertices;
			transformedVertices.resize(numVertices);
			for (size_t i = 0; i < numVertices; ++i)
				transformedVertices[i] = ((FrameworkVertex*)textured_detailed_sphere_vertices)[i];
			
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
					(FrameworkVertex*)textured_detailed_sphere_vertices,
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
				appendShape(compound->getChildShape(i), &compound->getChildTransform(i));
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

			FrameworkVertex vertices[4];

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
			btAlignedObjectArray<FrameworkVertex> vertices;
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

			btAlignedObjectArray<FrameworkVertex> vertices;
			btAlignedObjectArray<int> indices;
			
			if (pol != nullptr)
			{
				for (int v = 0; v < pol->m_vertices.size(); v++)
				{
					FrameworkVertex vertex;
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
						
						btTransform parentTransform;
						if (shapeTransform != nullptr)
							parentTransform = *shapeTransform;
						else
							parentTransform.setIdentity();
						
						const btVector3 pos0 = parentTransform * hull->getVertexPointer()[index0];
						const btVector3 pos1 = parentTransform * hull->getVertexPointer()[index1];
						const btVector3 pos2 = parentTransform * hull->getVertexPointer()[index2];
						
						btVector3 triNormal = (pos1 - pos0).cross(pos2 - pos0);
						triNormal.safeNormalize();

						for (int v = 0; v < 3; v++)
						{
							const int index = hull->getIndexPointer()[t * 3 + v];
							const btVector3 pos = parentTransform * hull->getVertexPointer()[index];
							
							FrameworkVertex vertex;
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
			logDebug("unknown collision shape type: %d", collisionShape->getShapeType());
			
			/*
			TRIANGLE_SHAPE_PROXYTYPE
			TETRAHEDRAL_SHAPE_PROXYTYPE
			CONE_SHAPE_PROXYTYPE
			CYLINDER_SHAPE_PROXYTYPE
			*/
		}
	};
	
	appendShape(collisionShape, nullptr);
	
	if (out_indices.size() > 0)
	{
		const int graphicsShapeId = m_renderInterface->registerShape(
			out_vertices[0].xyzw,
			out_vertices.size(),
			&out_indices[0],
			out_indices.size(),
			B3_GL_TRIANGLES,
			0);

		collisionShape->setUserIndex(graphicsShapeId);
	}
}

void FrameworkGUIHelperInterface::syncPhysicsToGraphics(const btDiscreteDynamicsWorld* rbWorld)
{
	const int numCollisionObjects = rbWorld->getNumCollisionObjects();
	
	{
		B3_PROFILE("write all InstanceTransformToCPU");
		
		for (int i = 0; i < numCollisionObjects; i++)
		{
			//B3_PROFILE("writeSingleInstanceTransformToCPU");
			
			const btCollisionObject * colObj = rbWorld->getCollisionObjectArray()[i];
			const btCollisionShape * collisionShape = colObj->getCollisionShape();
			
			if (collisionShape->getShapeType() == SOFTBODY_SHAPE_PROXYTYPE && collisionShape->getUserIndex() >= 0)
			{
				btAlignedObjectArray<FrameworkVertex> vertices;
				btAlignedObjectArray<int> indices;
				computeSoftBodyVertices(collisionShape, vertices, indices);
				if (vertices.size() > 0)
					m_renderInterface->updateShape(collisionShape->getUserIndex(), vertices[0].xyzw);
				continue;
			}
			
			const btVector3 & position = colObj->getWorldTransform().getOrigin();
			const btQuaternion rotation = colObj->getWorldTransform().getRotation();
			const int instanceId = colObj->getUserIndex();
			
			if (instanceId >= 0)
			{
				m_renderInterface->writeSingleInstanceTransformToCPU(position, rotation, instanceId);
			}
		}
	}
	{
		B3_PROFILE("writeTransforms");
		m_renderInterface->writeTransforms();
	}
}

void FrameworkGUIHelperInterface::syncPhysicsToGraphics2(const btDiscreteDynamicsWorld* rbWorld)
{
	Assert(false);
}

void FrameworkGUIHelperInterface::syncPhysicsToGraphics2(const GUISyncPosition* positions, int numPositions)
{
	Assert(false);
}

void FrameworkGUIHelperInterface::render(const btDiscreteDynamicsWorld* rbWorld)
{
	m_renderInterface->renderScene();
}

void FrameworkGUIHelperInterface::createPhysicsDebugDrawer(btDiscreteDynamicsWorld* rbWorld)
{
	btAssert(rbWorld);
	
	if (m_debugDraw != nullptr)
	{
		delete m_debugDraw;
		m_debugDraw = nullptr;
	}

	m_debugDraw = new FrameworkDebugDrawer();
	rbWorld->setDebugDrawer(m_debugDraw);

	m_debugDraw->setDebugMode(
		btIDebugDraw::DBG_DrawWireframe |
		btIDebugDraw::DBG_DrawAabb);
}

int FrameworkGUIHelperInterface::registerTexture(const unsigned char* texels, int width, int height)
{
	return m_renderInterface->registerTexture(texels, width, height);
}

int FrameworkGUIHelperInterface::registerGraphicsShape(const float* vertices, int numvertices, const int* indices, int numIndices, int primitiveType, int textureId)
{
	return m_renderInterface->registerShape(vertices, numvertices, indices, numIndices, primitiveType, textureId);
}

int FrameworkGUIHelperInterface::registerGraphicsInstance(int shapeId, const float* position, const float* quaternion, const float* color, const float* scaling)
{
	return m_renderInterface->registerGraphicsInstance(shapeId, position, quaternion, color, scaling);
}

void FrameworkGUIHelperInterface::removeAllGraphicsInstances()
{
	m_renderInterface->removeAllInstances();
}

void FrameworkGUIHelperInterface::removeGraphicsInstance(int graphicsId)
{
	m_renderInterface->removeGraphicsInstance(graphicsId);
}

void FrameworkGUIHelperInterface::changeRGBAColor(int instanceId, const double rgbaColor[4])
{
	m_renderInterface->writeSingleInstanceColorToCPU(rgbaColor, instanceId);
}

void FrameworkGUIHelperInterface::changeSpecularColor(int instanceId, const double specularColor[3])
{
	m_renderInterface->writeSingleInstanceSpecularColorToCPU(specularColor, instanceId);
}

void FrameworkGUIHelperInterface::changeTexture(int textureId, const unsigned char * texels, int width, int height)
{
	m_renderInterface->updateTexture(textureId, texels);
}

int FrameworkGUIHelperInterface::getShapeIndexFromInstance(int instanceId)
{
	return m_renderInterface->getShapeIndexFromInstance(instanceId);
}

void FrameworkGUIHelperInterface::replaceTexture(int shapeId, int textureId)
{
	m_renderInterface->replaceTexture(shapeId, textureId);
}

void FrameworkGUIHelperInterface::removeTexture(int id)
{
	m_renderInterface->removeTexture(id);
}

Common2dCanvasInterface* FrameworkGUIHelperInterface::get2dCanvasInterface()
{
	return m_2dCanvasInterface;
}

CommonParameterInterface* FrameworkGUIHelperInterface::getParameterInterface()
{
	return m_parameterInterface;
}

CommonRenderInterface* FrameworkGUIHelperInterface::getRenderInterface()
{
	return m_renderInterface;
}

const CommonRenderInterface* FrameworkGUIHelperInterface::getRenderInterface() const
{
	return m_renderInterface;
}

CommonGraphicsApp* FrameworkGUIHelperInterface::getAppInterface()
{
	return m_appInterface;
}

void FrameworkGUIHelperInterface::setUpAxis(int axis)
{
	m_appInterface->setUpAxis(axis);
}

void FrameworkGUIHelperInterface::resetCamera(float camDist, float yaw, float pitch, float camPosX, float camPosY, float camPosZ)
{
	if (getRenderInterface() && getRenderInterface()->getActiveCamera())
	{
		getRenderInterface()->getActiveCamera()->setCameraDistance(camDist);
		getRenderInterface()->getActiveCamera()->setCameraPitch(pitch);
		getRenderInterface()->getActiveCamera()->setCameraYaw(yaw);
		getRenderInterface()->getActiveCamera()->setCameraTargetPosition(camPosX, camPosY, camPosZ);
	}
}

bool FrameworkGUIHelperInterface::getCameraInfo(int* width, int* height, float viewMatrix[16], float projectionMatrix[16], float camUp[3], float camForward[3], float hor[3], float vert[3], float* yaw, float* pitch, float* camDist, float camTarget[3]) const
{
	Assert(false);
	return false;
}

void FrameworkGUIHelperInterface::setVisualizerFlag(int flag, int enable)
{
	Assert(false);
}

void FrameworkGUIHelperInterface::copyCameraImageData(
	const float viewMatrix[16], const float projectionMatrix[16],
	unsigned char* pixelsRGBA, int rgbaBufferSizeInPixels,
	float* depthBuffer, int depthBufferSizeInPixels,
	int startPixelIndex, int destinationWidth, int destinationHeight, int* numPixelsCopied)
{
	copyCameraImageData(
		viewMatrix, projectionMatrix, pixelsRGBA, rgbaBufferSizeInPixels,
		depthBuffer, depthBufferSizeInPixels,
		0, 0,
		startPixelIndex, destinationWidth,
		destinationHeight, numPixelsCopied);
}

void FrameworkGUIHelperInterface::copyCameraImageData(
	const float viewMatrix[16], const float projectionMatrix[16],
	unsigned char* pixelsRGBA, int rgbaBufferSizeInPixels,
	float* depthBuffer, int depthBufferSizeInPixels,
	int* segmentationMaskBuffer, int segmentationMaskBufferSizeInPixels,
	int startPixelIndex, int destinationWidth, int destinationHeight, int* numPixelsCopied)
{
	Assert(false);
}

void FrameworkGUIHelperInterface::debugDisplayCameraImageData(
	const float viewMatrix[16], const float projectionMatrix[16],
	 unsigned char* pixelsRGBA, int rgbaBufferSizeInPixels,
	 float* depthBuffer, int depthBufferSizeInPixels,
	 int* segmentationMaskBuffer, int segmentationMaskBufferSizeInPixels,
	 int startPixelIndex, int destinationWidth, int destinationHeight, int* numPixelsCopied)
{
	Assert(false);
}

void FrameworkGUIHelperInterface::setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16])
{
}

void FrameworkGUIHelperInterface::setProjectiveTexture(bool useProjectiveTexture)
{
}

static const btVector4 sColors[4] =
{
	btVector4(60. / 256., 186. / 256., 84. / 256., 1),
	btVector4(244. / 256., 194. / 256., 13. / 256., 1),
	btVector4(219. / 256., 50. / 256., 54. / 256., 1),
	btVector4(72. / 256., 133. / 256., 237. / 256., 1)
};

void FrameworkGUIHelperInterface::autogenerateGraphicsObjects(btDiscreteDynamicsWorld * rbWorld)
{
	for (int i = 0; i < rbWorld->getNumCollisionObjects(); i++)
	{
		btCollisionObject * colObj = rbWorld->getCollisionObjectArray()[i];
		
		// assign soft body ptr when this is a soft body object
		btSoftBody * sb = btSoftBody::upcast(colObj);
		if (sb != nullptr)
			colObj->getCollisionShape()->setUserPointer(sb);
	
		createCollisionShapeGraphicsObject(colObj->getCollisionShape());
		int colorIndex = colObj->getBroadphaseHandle()->getUid() & 3;

		btVector4 color = sColors[colorIndex];
		if (colObj->getCollisionShape()->getShapeType() == STATIC_PLANE_PROXYTYPE)
			color.setValue(1, 1, 1, 1);
		
		createCollisionObjectGraphicsObject(colObj, color);
	}
}

void FrameworkGUIHelperInterface::drawText3D(const char* txt, float posX, float posY, float posZ, float size)
{
	m_appInterface->drawText3D(txt, posX, posY, posZ, size);
}

void FrameworkGUIHelperInterface::drawText3D(const char* txt, float position[3], float orientation[4], float color[4], float size, int optionFlag)
{
	m_appInterface->drawText3D(txt, position, orientation, color, size, optionFlag);
}

int FrameworkGUIHelperInterface::addUserDebugText3D(const char* txt, const double positionXYZ[3], const double orientation[4], const double textColorRGB[3], double size, double lifeTime, int trackingVisualShapeIndex, int optionFlags, int replaceItemUid)
{
	Assert(false);
	return -1;
}

int FrameworkGUIHelperInterface::addUserDebugLine(const double debugLineFromXYZ[3], const double debugLineToXYZ[3], const double debugLineColorRGB[3], double lineWidth, double lifeTime, int trackingVisualShapeIndex, int replaceItemUid)
{
	Assert(false);
	return -1;
}

int FrameworkGUIHelperInterface::addUserDebugParameter(const char* txt, double rangeMin, double rangeMax, double startValue)
{
	return -1;
}

int FrameworkGUIHelperInterface::readUserDebugParameter(int itemUniqueId, double* value)
{
	return 0;
}

void FrameworkGUIHelperInterface::removeUserDebugItem(int debugItemUniqueId)
{
}

void FrameworkGUIHelperInterface::removeAllUserDebugItems()
{
}

void FrameworkGUIHelperInterface::setVisualizerFlagCallback(VisualizerFlagCallback callback)
{
}

void FrameworkGUIHelperInterface::dumpFramesToVideo(const char* mp4FileName)
{
}
