#pragma once

#include "btBulletDynamicsCommon.h" // world. todo : move to cpp

#include "framework.h"
#include "framework-camera.h"
#include "FrameworkDebugDrawer.h"
#include "FrameworkRenderInterface.h"

struct FrameworkGUIHelperInterface : GUIHelperInterface
{
	FrameworkDebugDrawer * m_debugDraw = nullptr;
	
// todo : just makes these globals. dear oh dear what a mess Bullit!
	CommonRenderInterface * m_renderInterface = nullptr;
	CommonGraphicsApp * m_appInterface = nullptr;
	
	virtual ~FrameworkGUIHelperInterface() override final
	{
	}

	virtual void createRigidBodyGraphicsObject(btRigidBody* body, const btVector3& color) override final
	{
		Assert(false);
	}

	virtual void createCollisionObjectGraphicsObject(btCollisionObject* obj, const btVector3& color) override final
	{
		Assert(false);
	}

	virtual void createCollisionShapeGraphicsObject(btCollisionShape* collisionShape) override final
	{
		Assert(false);
	}

	virtual void syncPhysicsToGraphics(const btDiscreteDynamicsWorld* rbWorld) override final
	{
		Assert(false);
	}
	
	virtual void syncPhysicsToGraphics2(const btDiscreteDynamicsWorld* rbWorld) override final
	{
		Assert(false);
	}
	
	virtual void syncPhysicsToGraphics2(const GUISyncPosition* positions, int numPositions) override final
	{
		Assert(false);
	}
	
	virtual void render(const btDiscreteDynamicsWorld* rbWorld) override final
	{
		Assert(false);
	}

	virtual void createPhysicsDebugDrawer(btDiscreteDynamicsWorld* rbWorld) override final
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
			btIDebugDraw::DBG_DrawWireframe + btIDebugDraw::DBG_DrawAabb
			//btIDebugDraw::DBG_DrawContactPoints
		);
	}

	virtual int registerTexture(const unsigned char* texels, int width, int height) override final
	{
		return createTextureFromRGBA8(texels, width, height, true, true);
	}
	
	virtual int registerGraphicsShape(const float* vertices, int numvertices, const int* indices, int numIndices, int primitiveType, int textureId) override final
	{
		Assert(false);
		return 0;
	}
	
	virtual int registerGraphicsInstance(int shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling) override final
	{
		Assert(false);
		return 0;
	}
	
	virtual void removeAllGraphicsInstances() override final
	{
		Assert(false);
	}
	
	virtual void removeGraphicsInstance(int graphicsUid) override final
	{
		Assert(false);
	}
	
	virtual void changeRGBAColor(int instanceUid, const double rgbaColor[4]) override final
	{
		Assert(false);
	}
	
	virtual void changeSpecularColor(int instanceUid, const double specularColor[3]) override final
	{
		Assert(false);
	}
	
	virtual void changeTexture(int textureUniqueId, const unsigned char* rgbTexels, int width, int height) override final
	{
		Assert(false);
	}

	virtual int getShapeIndexFromInstance(int instanceUid) override final
	{
		Assert(false);
		return -1;
	}
	
	virtual void replaceTexture(int shapeIndex, int textureUid) override final
	{
		Assert(false);
	}
	
	virtual void removeTexture(int textureUid) override final
	{
		GxTextureId temp = textureUid;
		freeTexture(temp);
	}

	virtual Common2dCanvasInterface* get2dCanvasInterface() override final
	{
		Assert(false);
		return 0;
	}

	virtual CommonParameterInterface* getParameterInterface() override final
	{
		Assert(false);
		return 0;
	}

	virtual CommonRenderInterface* getRenderInterface() override final
	{
		return m_renderInterface;
	}

	virtual const CommonRenderInterface* getRenderInterface() const override final
	{
		return m_renderInterface;
	}
	
	virtual CommonGraphicsApp* getAppInterface() override final
	{
		return m_appInterface;
	}

	virtual void setUpAxis(int axis) override final
	{
		m_appInterface->setUpAxis(axis);
	}

	virtual void resetCamera(float camDist, float yaw, float pitch, float camPosX, float camPosY, float camPosZ) override final
	{
		if (getRenderInterface() && getRenderInterface()->getActiveCamera())
		{
			getRenderInterface()->getActiveCamera()->setCameraDistance(camDist);
			getRenderInterface()->getActiveCamera()->setCameraPitch(pitch);
			getRenderInterface()->getActiveCamera()->setCameraYaw(yaw);
			getRenderInterface()->getActiveCamera()->setCameraTargetPosition(camPosX, camPosY, camPosZ);
		}
	}

	virtual bool getCameraInfo(int* width, int* height, float viewMatrix[16], float projectionMatrix[16], float camUp[3], float camForward[3], float hor[3], float vert[3], float* yaw, float* pitch, float* camDist, float camTarget[3]) const override final
	{
		Assert(false);
		return false;
	}
	
	virtual void setVisualizerFlag(int flag, int enable) override final
	{
		Assert(false);
	}

	virtual void copyCameraImageData(
		const float viewMatrix[16], const float projectionMatrix[16],
		unsigned char* pixelsRGBA, int rgbaBufferSizeInPixels,
		float* depthBuffer, int depthBufferSizeInPixels,
		int startPixelIndex, int destinationWidth, int destinationHeight, int* numPixelsCopied) override final
	{
		copyCameraImageData(
			viewMatrix, projectionMatrix, pixelsRGBA, rgbaBufferSizeInPixels,
			depthBuffer, depthBufferSizeInPixels,
			0, 0,
			startPixelIndex, destinationWidth,
			destinationHeight, numPixelsCopied);
	}

	virtual void copyCameraImageData(
		const float viewMatrix[16], const float projectionMatrix[16],
		unsigned char* pixelsRGBA, int rgbaBufferSizeInPixels,
		float* depthBuffer, int depthBufferSizeInPixels,
		int* segmentationMaskBuffer, int segmentationMaskBufferSizeInPixels,
		int startPixelIndex, int destinationWidth, int destinationHeight, int* numPixelsCopied) override final
	{
		Assert(false);
	}
	
	virtual void debugDisplayCameraImageData(
		const float viewMatrix[16], const float projectionMatrix[16],
		 unsigned char* pixelsRGBA, int rgbaBufferSizeInPixels,
		 float* depthBuffer, int depthBufferSizeInPixels,
		 int* segmentationMaskBuffer, int segmentationMaskBufferSizeInPixels,
		 int startPixelIndex, int destinationWidth, int destinationHeight, int* numPixelsCopied) override final
	 {
	 	Assert(false);
	 }

	virtual void setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16]) override { }
	virtual void setProjectiveTexture(bool useProjectiveTexture) override { }

	virtual void autogenerateGraphicsObjects(btDiscreteDynamicsWorld* rbWorld) override final
	{
	}

	virtual void drawText3D(const char* txt, float posX, float posY, float posZ, float size) override
	{
		Assert(false);
	}
	
	virtual void drawText3D(const char* txt, float position[3], float orientation[4], float color[4], float size, int optionFlag) override
	{
		Assert(false);
	}

	virtual int addUserDebugText3D(const char* txt, const double positionXYZ[3], const double orientation[4], const double textColorRGB[3], double size, double lifeTime, int trackingVisualShapeIndex, int optionFlags, int replaceItemUid) override
	{
		Assert(false);
		return -1;
	}
	
	virtual int addUserDebugLine(const double debugLineFromXYZ[3], const double debugLineToXYZ[3], const double debugLineColorRGB[3], double lineWidth, double lifeTime, int trackingVisualShapeIndex, int replaceItemUid) override
	{
		Assert(false);
		return -1;
	}
	
	virtual int addUserDebugParameter(const char* txt, double rangeMin, double rangeMax, double startValue) override { return -1; };
	virtual int readUserDebugParameter(int itemUniqueId, double* value) override { return 0; }

	virtual void removeUserDebugItem(int debugItemUniqueId) override { };
	virtual void removeAllUserDebugItems() override { };
	virtual void setVisualizerFlagCallback(VisualizerFlagCallback callback) override { }

	//empty name stops dumping video
	virtual void dumpFramesToVideo(const char* mp4FileName) override { };
};
