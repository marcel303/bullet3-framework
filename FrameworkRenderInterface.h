#pragma once

#include "CommonInterfaces/CommonRenderInterface.h"
#include "OpenGLWindow/SimpleCamera.h"
#include "Vec3.h"
#include <map>

struct GxTexture;

class Mat4x4;
class ShaderBuffer;

struct FrameworkGraphicsInstance;
struct FrameworkGraphicsShape;

class FrameworkRenderInterface : public CommonRenderInterface
{
private:
	SimpleCamera camera;
	
	int m_screenWidth = 0;
	int m_screenHeight = 0;
	
	struct
	{
		Vec3 position;
	} light;
	
	ShaderBuffer * transformsBuffer = nullptr;
	ShaderBuffer * colorsBuffer = nullptr;
	
	std::map<int, FrameworkGraphicsShape*> m_graphicsShapes;
	int m_nextGraphicsShapeId = 1;

	std::map<int, FrameworkGraphicsInstance*> m_graphicsInstances;
	int m_nextGraphicsInstanceId = 1;

	std::map<int, GxTexture*> m_textures;
	int m_nextTextureId = 1;
	
	FrameworkGraphicsInstance * resolveGraphicsInstance(int id) const;
	
	void calculateViewMatrix(Mat4x4 & result) const;
	
public:
	virtual ~FrameworkRenderInterface() override final;
	
	virtual void init() override final;
	
	virtual void updateCamera(int upAxis) override final;
	
	virtual void removeAllInstances() override final;
	virtual void removeGraphicsInstance(int instanceId) override final;

	virtual const CommonCameraInterface* getActiveCamera() const override final;
	
	virtual CommonCameraInterface* getActiveCamera() override final;
	
	virtual void setActiveCamera(CommonCameraInterface* cam) override final;

	virtual void setLightPosition(const float lightPos[3]) override final;
	virtual void setLightPosition(const double lightPos[3]) override final;
	virtual void setShadowMapResolution(int shadowMapResolution) override final;
	virtual void setShadowMapWorldSize(float worldSize) override final;
	
	virtual void setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16]) override;
	virtual void setProjectiveTexture(bool useProjectiveTexture) override;

	virtual void renderScene() override final;
	virtual void renderSceneInternal(int renderMode = B3_DEFAULT_RENDERMODE) override;
	
	virtual int getScreenWidth() override final;
	virtual int getScreenHeight() override final;

	virtual void resize(int width, int height) override final;

	virtual int registerGraphicsInstance(int shapeId, const float* position, const float* quaternion, const float* color, const float* scaling) override final;
	virtual int registerGraphicsInstance(int shapeId, const double* position, const double* quaternion, const double* color, const double* scaling) override final;
	virtual void drawLines(const float* positions, const float color[4], int numPoints, int pointStrideInBytes, const unsigned int* indices, int numIndices, float pointDrawSize) override final;
	
	virtual void drawLine(const float from[4], const float to[4], const float color[4], float lineWidth) override final;
	
	virtual void drawLine(const double from[4], const double to[4], const double color[4], double lineWidth) override final;
	
	virtual void drawPoint(const float* position, const float color[4], float pointDrawSize) override final;
	
	virtual void drawPoint(const double* position, const double color[4], double pointDrawSize) override final;
	
	virtual void drawTexturedTriangleMesh(float worldPosition[3], float worldOrientation[4], const float* vertices, int numvertices, const unsigned int* indices, int numIndices, float color[4], int textureId = -1, int vertexLayout = 0) override final;

	virtual int registerShape(const float* vertices, int numvertices, const int* indices, int numIndices, int primitiveType = B3_GL_TRIANGLES, int textureId = -1) override final;
	virtual void updateShape(int shapeId, const float* vertices) override final;

	virtual int registerTexture(const unsigned char* texels, int width, int height, bool flipPixelsY = true) override final;
	virtual void updateTexture(int textureId, const unsigned char* texels, bool flipPixelsY = true) override final;
	virtual void activateTexture(int textureId) override final;
	virtual void replaceTexture(int shapeId, int textureId) override final;
	virtual void removeTexture(int textureId) override final;

	virtual void setPlaneReflectionShapeIndex(int index) override final;
	
	virtual int getShapeIndexFromInstance(int instanceId) override final;

	virtual bool readSingleInstanceTransformToCPU(float* position, float* orientation, int srcIndex) override final;
	virtual void writeSingleInstanceTransformToCPU(const float* position, const float* orientation, int srcIndex) override final;
	virtual void writeSingleInstanceTransformToCPU(const double* position, const double* orientation, int srcIndex) override final;
	virtual void writeSingleInstanceColorToCPU(const float* color, int srcIndex) override final;
	
	virtual void writeSingleInstanceColorToCPU(const double* color, int srcIndex) override final;
	virtual void writeSingleInstanceScaleToCPU(const float* scale, int srcIndex) override final;
	virtual void writeSingleInstanceScaleToCPU(const double* scale, int srcIndex) override final;
	virtual void writeSingleInstanceSpecularColorToCPU(const double* specular, int srcIndex) override final;
	virtual void writeSingleInstanceSpecularColorToCPU(const float* specular, int srcIndex) override final;
	virtual void writeSingleInstanceFlagsToCPU(int flags, int srcIndex) override final;
	
	virtual int getTotalNumInstances() const override final;
	
	virtual void writeTransforms() override final;

	virtual void clearZBuffer() override final;

	virtual struct GLInstanceRendererInternalData * getInternalData() override final;
};
