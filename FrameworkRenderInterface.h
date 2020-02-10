#pragma once

#include "CommonInterfaces/CommonRenderInterface.h"

#include "Debugging.h" // todo : move to cpp
#include "framework.h" // todo : move to cpp
#include "OpenGLWindow/SimpleCamera.h"

#include "framework-camera.h" // todo : remove

class FrameworkRenderInterface : public CommonRenderInterface
{
	SimpleCamera camera;
	
	int m_screenWidth = 0;
	int m_screenHeight = 0;
	
public:
	virtual ~FrameworkRenderInterface() override final
	{
	}
	
	virtual void init() override final { }
	
	virtual void updateCamera(int upAxis) override final
	{
		//Assert(false);
		Mat4x4 viewMatrix;
		camera.getCameraViewMatrix(viewMatrix.m_v);
		viewMatrix = Mat4x4(true).Scale(1, 1, -1).Mul(viewMatrix);
		gxSetMatrixf(GX_MODELVIEW, viewMatrix.m_v);
	}
	
	virtual void removeAllInstances() override final { }
	virtual void removeGraphicsInstance(int instanceUid) override final { }

	virtual const CommonCameraInterface* getActiveCamera() const override final
	{
		return &camera;
	}
	
	virtual CommonCameraInterface* getActiveCamera() override final
	{
		return &camera;
	}
	
	virtual void setActiveCamera(CommonCameraInterface* cam) override final
	{
		Assert(false);
	}

	virtual void setLightPosition(const float lightPos[3]) override final { }
	virtual void setLightPosition(const double lightPos[3]) override final { }
	virtual void setShadowMapResolution(int shadowMapResolution) override final { }
	virtual void setShadowMapWorldSize(float worldSize) override final { }
	
	virtual void setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16]) override { };
	virtual void setProjectiveTexture(bool useProjectiveTexture) override { };

	virtual void renderScene() override final { }
	virtual void renderSceneInternal(int renderMode = B3_DEFAULT_RENDERMODE) override { };
	
	virtual int getScreenWidth() override final
	{
		return m_screenWidth;
	}
	
	virtual int getScreenHeight() override final
	{
		return m_screenHeight;
	}

	virtual void resize(int width, int height) override final
	{
		m_screenWidth = width;
		m_screenHeight = height;
	}

	virtual int registerGraphicsInstance(int shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling) override final { return 0; }
	virtual int registerGraphicsInstance(int shapeIndex, const double* position, const double* quaternion, const double* color, const double* scaling) override final { return 0; }
	virtual void drawLines(const float* positions, const float color[4], int numPoints, int pointStrideInBytes, const unsigned int* indices, int numIndices, float pointDrawSize) override final
	{
		Assert(false);
	}
	
	virtual void drawLine(const float from[4], const float to[4], const float color[4], float lineWidth) override final
	{
		gxBegin(GX_LINES);
		{
			gxColor4fv(color);
			gxVertex4fv(from);
			gxVertex4fv(to);
		}
		gxEnd();
	}
	
	virtual void drawLine(const double from[4], const double to[4], const double color[4], double lineWidth) override final
	{
		gxBegin(GX_LINES);
		{
			gxColor4f(color[0], color[1], color[2], color[3]);
			gxVertex4f(from[0], from[1], from[2], from[3]);
			gxVertex4f(to[0], to[1], to[2], to[3]);
		}
		gxEnd();
	}
	
	virtual void drawPoint(const float* position, const float color[4], float pointDrawSize) override final
	{
		Assert(false);
	}
	
	virtual void drawPoint(const double* position, const double color[4], double pointDrawSize) override final
	{
		Assert(false);
	}
	
	virtual void drawTexturedTriangleMesh(float worldPosition[3], float worldOrientation[4], const float* vertices, int numvertices, const unsigned int* indices, int numIndices, float color[4], int textureIndex = -1, int vertexLayout = 0) override final
	{
		Assert(false);
	}

	virtual int registerShape(const float* vertices, int numvertices, const int* indices, int numIndices, int primitiveType = B3_GL_TRIANGLES, int textureIndex = -1) override final
	{
		Assert(false);
		return 0;
	}
	
	virtual void updateShape(int shapeIndex, const float* vertices) override final
	{
		Assert(false);
	}

	virtual int registerTexture(const unsigned char* texels, int width, int height, bool flipPixelsY = true) override final
	{
		Assert(false);
		return 0;
	}
	
	virtual void updateTexture(int textureIndex, const unsigned char* texels, bool flipPixelsY = true) override final
	{
		Assert(false);
	}
	
	virtual void activateTexture(int textureIndex) override final
	{
		Assert(false);
	}
	
	virtual void replaceTexture(int shapeIndex, int textureIndex) override final
	{
		Assert(false);
	}
	
	virtual void removeTexture(int textureIndex) override final
	{
		Assert(false);
	}

	virtual void setPlaneReflectionShapeIndex(int index) override final
	{
		Assert(false);
	}

	virtual int getShapeIndexFromInstance(int srcIndex) override final
	{
		return -1;
	}

	virtual bool readSingleInstanceTransformToCPU(float* position, float* orientation, int srcIndex) override final
	{
		Assert(false);
		return false;
	}

	virtual void writeSingleInstanceTransformToCPU(const float* position, const float* orientation, int srcIndex) override final
	{
		Assert(false);
	}
	
	virtual void writeSingleInstanceTransformToCPU(const double* position, const double* orientation, int srcIndex) override final
	{
		Assert(false);
	}
	
	virtual void writeSingleInstanceColorToCPU(const float* color, int srcIndex) override final
	{
		Assert(false);
	}
	
	virtual void writeSingleInstanceColorToCPU(const double* color, int srcIndex) override final
	{
		Assert(false);
	}
	
	virtual void writeSingleInstanceScaleToCPU(const float* scale, int srcIndex) override final
	{
		Assert(false);
	}
	
	virtual void writeSingleInstanceScaleToCPU(const double* scale, int srcIndex) override final
	{
		Assert(false);
	}
	
	virtual void writeSingleInstanceSpecularColorToCPU(const double* specular, int srcIndex) override final
	{
		Assert(false);
	}
	
	virtual void writeSingleInstanceSpecularColorToCPU(const float* specular, int srcIndex) override final
	{
		Assert(false);
	}

	virtual int getTotalNumInstances() const override final
	{
		Assert(false);
		return 0;
	}

	virtual void writeTransforms() override final
	{
		Assert(false);
	}

	virtual void clearZBuffer() override final
	{
		Assert(false);
	}

	//This is internal access to OpenGL3+ features, mainly used for OpenCL-OpenGL interop
	//Only the GLInstancingRenderer supports it, just return 0 otherwise.
	virtual struct GLInstanceRendererInternalData * getInternalData() override final
	{
		return nullptr;
	}
};
