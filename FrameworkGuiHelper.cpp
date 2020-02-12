#include "FrameworkDebugDrawer.h"
#include "FrameworkGuiHelper.h"

#include "CommonInterfaces/CommonGraphicsAppInterface.h"
#include "CommonInterfaces/CommonRenderInterface.h"

#include "btBulletDynamicsCommon.h"

#include "framework.h"

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
		// the graphics shape is already scaled
		const btVector3 localScaling(1, 1, 1);
		
		const int graphicsInstanceId = m_renderInterface->registerGraphicsInstance(
			graphicsShapeId,
			startTransform.getOrigin(),
			startTransform.getRotation(),
			color,
			localScaling);
		
		body->setUserIndex(graphicsInstanceId);
	}
}

#include "OpenGLWindow/ShapeData.h"

static void appendPrim(
	const float * vertices,
	const int numVertices,
	const int * indices,
	const int numIndices,
	const btTransform * transform,
	const btVector3 & halfExtents,
	btAlignedObjectArray<float> & out_vertices,
	btAlignedObjectArray<int> & out_indices)
{
	const int baseVertex = out_vertices.size() / 9;
	
	out_vertices.resize(out_vertices.size() + numVertices * 9);
	for (int i = 0; i < numVertices * 9; ++i)
		out_vertices[baseVertex*9 + i] = vertices[i];

	for (int i = 0; i < numVertices; ++i)
	{
		float & x = out_vertices[(baseVertex + i) * 9 + 0];
		float & y = out_vertices[(baseVertex + i) * 9 + 1];
		float & z = out_vertices[(baseVertex + i) * 9 + 2];
		
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

void FrameworkGUIHelperInterface::createCollisionShapeGraphicsObject(btCollisionShape* collisionShape)
{
	// already has a graphics object ?
	
	if (collisionShape->getUserIndex() >= 0)
		return;
	
	const int vertexStrideInBytes = 9 * sizeof(float);

	btAlignedObjectArray<float> out_vertices;
	btAlignedObjectArray<int> out_indices;
	
	if (collisionShape->getShapeType() == BOX_SHAPE_PROXYTYPE)
	{
		const btBoxShape * boxShape = (btBoxShape*)collisionShape;
		const btVector3 halfExtents = boxShape->getHalfExtentsWithMargin();
		
		appendPrim(
			cube_vertices_textured,
			sizeof(cube_vertices_textured) / vertexStrideInBytes,
			cube_indices,
			sizeof(cube_indices) / sizeof(cube_indices[0]),
			nullptr,
			halfExtents,
			out_vertices,
			out_indices);
	}
	else if (collisionShape->getShapeType() == SPHERE_SHAPE_PROXYTYPE)
	{
		const btSphereShape * sphereShape = (btSphereShape*)collisionShape;
		const btScalar sphereSize = 2. * sphereShape->getRadius();

		appendPrim(
			textured_detailed_sphere_vertices,
			sizeof(textured_detailed_sphere_vertices) / vertexStrideInBytes,
			textured_detailed_sphere_indices,
			sizeof(textured_detailed_sphere_indices) / sizeof(textured_detailed_sphere_indices[0]),
			nullptr,
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
		
		btAlignedObjectArray<float> transformedVertices;
		transformedVertices.resize(numVertices * 9);
		for (size_t i = 0; i < transformedVertices.size(); ++i)
			transformedVertices[i] = textured_detailed_sphere_vertices[i];
		
		for (int i = 0; i < numVertices; ++i)
		{
			transformedVertices[i * 9 + 0] *= radiusScale.x();
			transformedVertices[i * 9 + 1] *= radiusScale.y();
			transformedVertices[i * 9 + 2] *= radiusScale.z();
			
			if (transformedVertices[i * 9 + up] > 0)
				transformedVertices[i * 9 + up] += halfHeight;
			else
				transformedVertices[i * 9 + up] -= halfHeight;
		}
		
		const int graphicsShapeId = m_renderInterface->registerShape(
			&transformedVertices[0],
			numVertices,
			indices,
			numIndices,
			B3_GL_TRIANGLES,
			0);

		collisionShape->setUserIndex(graphicsShapeId);
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
			
			appendPrim(
				textured_detailed_sphere_vertices,
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
		const btCompoundShape * compound = (btCompoundShape*)collisionShape;
		
		for (int i = 0; i < compound->getNumChildShapes(); ++i)
		{
			const btCollisionShape * childShape = compound->getChildShape(i);
			
			if (childShape->getShapeType() == BOX_SHAPE_PROXYTYPE)
			{
				const btBoxShape * boxShape = (btBoxShape*)childShape;
				const btVector3 halfExtents = boxShape->getHalfExtentsWithMargin();
				
				appendPrim(
					cube_vertices_textured,
					sizeof(cube_vertices_textured) / vertexStrideInBytes,
					cube_indices,
					sizeof(cube_indices) / sizeof(cube_indices[0]),
					&compound->getChildTransform(i),
					halfExtents,
					out_vertices,
					out_indices);
			}
			else if (childShape->getShapeType() == SPHERE_SHAPE_PROXYTYPE)
			{
				const btSphereShape * sphereShape = (btSphereShape*)childShape;
				const btScalar sphereSize = 2.f * sphereShape->getRadius();

				appendPrim(
					textured_detailed_sphere_vertices,
					sizeof(textured_detailed_sphere_vertices) / vertexStrideInBytes,
					textured_detailed_sphere_indices,
					sizeof(textured_detailed_sphere_indices) / sizeof(textured_detailed_sphere_indices[0]),
					&compound->getChildTransform(i),
					btVector3(sphereSize, sphereSize, sphereSize),
					out_vertices,
					out_indices);
			}
		}
	}
	
	if (out_indices.size() > 0)
	{
		const int graphicsShapeId = m_renderInterface->registerShape(
			&out_vertices[0],
			out_vertices.size() / 9,
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
			
			/*
			if (collisionShape->getShapeType() == SOFTBODY_SHAPE_PROXYTYPE && collisionShape->getUserIndex() >= 0)
			{
				btAlignedObjectArray<GLInstanceVertex> gfxVertices;
				btAlignedObjectArray<int> indices;
				computeSoftBodyVertices(collisionShape, gfxVertices, indices);
				m_renderInterface->updateShape(collisionShape->getUserIndex(), &gfxVertices[0].xyzw[0]);
				continue;
			}
			*/
			
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
		btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawAabb
		//btIDebugDraw::DBG_DrawContactPoints
	);
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
	// todo : what about width and height ?
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
	btVector4(72. / 256., 133. / 256., 237. / 256., 1),
	//btVector4(1,1,0,1),
};

static bool shapePointerCompareFunc(const btCollisionObject * colA, const btCollisionObject * colB)
{
	auto * a = colA->getCollisionShape();
	auto * b = colB->getCollisionShape();
	return (uintptr_t)a < (uintptr_t)b;
}

void FrameworkGUIHelperInterface::autogenerateGraphicsObjects(btDiscreteDynamicsWorld * rbWorld)
{
	// sort the collision objects based on collision shape, the gfx library requires instances that re-use a shape to be added after eachother

	btAlignedObjectArray<btCollisionObject*> sortedObjects;
	sortedObjects.reserve(rbWorld->getNumCollisionObjects());
	
	for (int i = 0; i < rbWorld->getNumCollisionObjects(); i++)
	{
		btCollisionObject * colObj = rbWorld->getCollisionObjectArray()[i];
		sortedObjects.push_back(colObj);
	}
	
	sortedObjects.quickSort(shapePointerCompareFunc);
	
	for (int i = 0; i < sortedObjects.size(); i++)
	{
		btCollisionObject * colObj = sortedObjects[i];
		
	#if false // todo : why ?
		//btRigidBody* body = btRigidBody::upcast(colObj);
		//does this also work for btMultiBody/btMultiBodyLinkCollider?
		btSoftBody* sb = btSoftBody::upcast(colObj);
		if (sb)
		{
			colObj->getCollisionShape()->setUserPointer(sb);
		}
	#endif
	
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
