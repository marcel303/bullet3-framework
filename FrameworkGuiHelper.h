#pragma once

#include "CommonInterfaces/CommonGUIHelperInterface.h"

struct CommonGraphicsApp;
struct CommonRenderInterface;

class FrameworkDebugDrawer;

struct FrameworkGUIHelperInterface : GUIHelperInterface
{
	FrameworkDebugDrawer * m_debugDraw = nullptr;
	
// todo : just makes these globals. dear oh dear what a mess Bullit!
	CommonRenderInterface * m_renderInterface = nullptr;
	CommonGraphicsApp * m_appInterface = nullptr;
	
	virtual ~FrameworkGUIHelperInterface() override final;

	virtual void createRigidBodyGraphicsObject(btRigidBody* body, const btVector3& color) override final;
	virtual void createCollisionObjectGraphicsObject(btCollisionObject* obj, const btVector3& color) override final;
	virtual void createCollisionShapeGraphicsObject(btCollisionShape* collisionShape) override final;

	virtual void syncPhysicsToGraphics(const btDiscreteDynamicsWorld* rbWorld) override final;
	virtual void syncPhysicsToGraphics2(const btDiscreteDynamicsWorld* rbWorld) override final;
	virtual void syncPhysicsToGraphics2(const GUISyncPosition* positions, int numPositions) override final;
	
	virtual void render(const btDiscreteDynamicsWorld* rbWorld) override final;

	virtual void createPhysicsDebugDrawer(btDiscreteDynamicsWorld* rbWorld) override final;

	virtual int registerTexture(const unsigned char* texels, int width, int height) override final;
	virtual int registerGraphicsShape(const float* vertices, int numvertices, const int* indices, int numIndices, int primitiveType, int textureId) override final;
	virtual int registerGraphicsInstance(int shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling) override final;
	
	virtual void removeAllGraphicsInstances() override final;
	virtual void removeGraphicsInstance(int graphicsUid) override final;
	
	virtual void changeRGBAColor(int instanceUid, const double rgbaColor[4]) override final;
	
	virtual void changeSpecularColor(int instanceUid, const double specularColor[3]) override final;
	virtual void changeTexture(int textureUniqueId, const unsigned char* rgbTexels, int width, int height) override final;

	virtual int getShapeIndexFromInstance(int instanceUid) override final;
	
	virtual void replaceTexture(int shapeIndex, int textureUid) override final;
	virtual void removeTexture(int textureUid) override final;
	
	virtual Common2dCanvasInterface* get2dCanvasInterface() override final;
	virtual CommonParameterInterface* getParameterInterface() override final;
	virtual CommonRenderInterface* getRenderInterface() override final;
	virtual const CommonRenderInterface* getRenderInterface() const override final;
	virtual CommonGraphicsApp* getAppInterface() override final;
	
	virtual void setUpAxis(int axis) override final;
	virtual void resetCamera(float camDist, float yaw, float pitch, float camPosX, float camPosY, float camPosZ) override final;
	virtual bool getCameraInfo(int* width, int* height, float viewMatrix[16], float projectionMatrix[16], float camUp[3], float camForward[3], float hor[3], float vert[3], float* yaw, float* pitch, float* camDist, float camTarget[3]) const override final;
	
	virtual void setVisualizerFlag(int flag, int enable) override final;

	virtual void copyCameraImageData(
		const float viewMatrix[16], const float projectionMatrix[16],
		unsigned char* pixelsRGBA, int rgbaBufferSizeInPixels,
		float* depthBuffer, int depthBufferSizeInPixels,
		int startPixelIndex, int destinationWidth, int destinationHeight, int* numPixelsCopied) override final;
	virtual void copyCameraImageData(
		const float viewMatrix[16], const float projectionMatrix[16],
		unsigned char* pixelsRGBA, int rgbaBufferSizeInPixels,
		float* depthBuffer, int depthBufferSizeInPixels,
		int* segmentationMaskBuffer, int segmentationMaskBufferSizeInPixels,
		int startPixelIndex, int destinationWidth, int destinationHeight, int* numPixelsCopied) override final;
	virtual void debugDisplayCameraImageData(
		const float viewMatrix[16], const float projectionMatrix[16],
		 unsigned char* pixelsRGBA, int rgbaBufferSizeInPixels,
		 float* depthBuffer, int depthBufferSizeInPixels,
		 int* segmentationMaskBuffer, int segmentationMaskBufferSizeInPixels,
		 int startPixelIndex, int destinationWidth, int destinationHeight, int* numPixelsCopied) override final;

	virtual void setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16]) override final;
	virtual void setProjectiveTexture(bool useProjectiveTexture) override final;

	virtual void autogenerateGraphicsObjects(btDiscreteDynamicsWorld* rbWorld) override final;
	
	virtual void drawText3D(const char* txt, float posX, float posY, float posZ, float size) override final;
	virtual void drawText3D(const char* txt, float position[3], float orientation[4], float color[4], float size, int optionFlag) override final;
	
	virtual int addUserDebugText3D(const char* txt, const double positionXYZ[3], const double orientation[4], const double textColorRGB[3], double size, double lifeTime, int trackingVisualShapeIndex, int optionFlags, int replaceItemUid) override final;
	
	virtual int addUserDebugLine(const double debugLineFromXYZ[3], const double debugLineToXYZ[3], const double debugLineColorRGB[3], double lineWidth, double lifeTime, int trackingVisualShapeIndex, int replaceItemUid) override final;
	
	virtual int addUserDebugParameter(const char* txt, double rangeMin, double rangeMax, double startValue) override final;
	virtual int readUserDebugParameter(int itemUniqueId, double* value) override final;
	
	virtual void removeUserDebugItem(int debugItemUniqueId) override final;
	virtual void removeAllUserDebugItems() override final;
	
	virtual void setVisualizerFlagCallback(VisualizerFlagCallback callback) override final;
	
	virtual void dumpFramesToVideo(const char* mp4FileName) override final;
};
