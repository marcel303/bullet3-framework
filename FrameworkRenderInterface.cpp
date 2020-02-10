#include "FrameworkRenderInterface.h"

#include "Debugging.h"
#include "framework.h"

FrameworkRenderInterface::~FrameworkRenderInterface()
{
}

void FrameworkRenderInterface::init()
{
}

void FrameworkRenderInterface::updateCamera(int upAxis)
{
	Mat4x4 viewMatrix;
	camera.getCameraViewMatrix(viewMatrix.m_v);
	viewMatrix = Mat4x4(true).Scale(1, 1, -1).Mul(viewMatrix);
	gxSetMatrixf(GX_MODELVIEW, viewMatrix.m_v);
}

void FrameworkRenderInterface::removeAllInstances()
{
}

void FrameworkRenderInterface::removeGraphicsInstance(int instanceUid)
{
}

const CommonCameraInterface * FrameworkRenderInterface::getActiveCamera() const
{
	return &camera;
}

CommonCameraInterface * FrameworkRenderInterface::getActiveCamera()
{
	return &camera;
}

void FrameworkRenderInterface::setActiveCamera(CommonCameraInterface * cam)
{
	Assert(false);
}

void FrameworkRenderInterface::setLightPosition(const float lightPos[3]) { }
void FrameworkRenderInterface::setLightPosition(const double lightPos[3]) { }
void FrameworkRenderInterface::setShadowMapResolution(int shadowMapResolution) { }
void FrameworkRenderInterface::setShadowMapWorldSize(float worldSize) { }

void FrameworkRenderInterface::setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16]) { }
void FrameworkRenderInterface::setProjectiveTexture(bool useProjectiveTexture) { }

void FrameworkRenderInterface::renderScene() { }
void FrameworkRenderInterface::renderSceneInternal(int renderMode) { }

int FrameworkRenderInterface::getScreenWidth()
{
	return m_screenWidth;
}

int FrameworkRenderInterface::getScreenHeight()
{
	return m_screenHeight;
}

void FrameworkRenderInterface::resize(int width, int height)
{
	m_screenWidth = width;
	m_screenHeight = height;
}

int FrameworkRenderInterface::registerGraphicsInstance(int shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling) { return 0; }
int FrameworkRenderInterface::registerGraphicsInstance(int shapeIndex, const double* position, const double* quaternion, const double* color, const double* scaling) { return 0; }

void FrameworkRenderInterface::drawLines(const float* positions, const float color[4], int numPoints, int pointStrideInBytes, const unsigned int* indices, int numIndices, float pointDrawSize)
{
	gxColor4fv(color);
	gxBegin(GX_LINES);
	{
		for (int i = 0; i < numIndices; ++i)
		{
			const int index = indices[i];
			
			const float * position = (float*)(((uint8_t*)positions) + index * pointStrideInBytes);
			
			gxVertex4fv(position);
		}
	}
	gxEnd();
}

void FrameworkRenderInterface::drawLine(const float from[4], const float to[4], const float color[4], float lineWidth)
{
	gxBegin(GX_LINES);
	{
		gxColor4fv(color);
		gxVertex4fv(from);
		gxVertex4fv(to);
	}
	gxEnd();
}

void FrameworkRenderInterface::drawLine(const double from[4], const double to[4], const double color[4], double lineWidth)
{
	gxBegin(GX_LINES);
	{
		gxColor4f(color[0], color[1], color[2], color[3]);
		gxVertex4f(from[0], from[1], from[2], from[3]);
		gxVertex4f(to[0], to[1], to[2], to[3]);
	}
	gxEnd();
}

void FrameworkRenderInterface::drawPoint(const float* position, const float color[4], float pointDrawSize)
{
	Assert(false);
}

void FrameworkRenderInterface::drawPoint(const double* position, const double color[4], double pointDrawSize)
{
	Assert(false);
}

void FrameworkRenderInterface::drawTexturedTriangleMesh(float worldPosition[3], float worldOrientation[4], const float* vertices, int numvertices, const unsigned int* indices, int numIndices, float color[4], int textureIndex, int vertexLayout)
{
	Assert(false);
}

int FrameworkRenderInterface::registerShape(const float* vertices, int numvertices, const int* indices, int numIndices, int primitiveType, int textureIndex)
{
	Assert(false);
	return 0;
}

void FrameworkRenderInterface::updateShape(int shapeIndex, const float* vertices)
{
	Assert(false);
}

int FrameworkRenderInterface::registerTexture(const unsigned char* texels, int width, int height, bool flipPixelsY)
{
	Assert(false);
	return 0;
}

void FrameworkRenderInterface::updateTexture(int textureIndex, const unsigned char* texels, bool flipPixelsY)
{
	Assert(false);
}

void FrameworkRenderInterface::activateTexture(int textureIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::replaceTexture(int shapeIndex, int textureIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::removeTexture(int textureIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::setPlaneReflectionShapeIndex(int index)
{
	Assert(false);
}

int FrameworkRenderInterface::getShapeIndexFromInstance(int srcIndex)
{
	return -1;
}

bool FrameworkRenderInterface::readSingleInstanceTransformToCPU(float* position, float* orientation, int srcIndex)
{
	Assert(false);
	return false;
}

void FrameworkRenderInterface::writeSingleInstanceTransformToCPU(const float* position, const float* orientation, int srcIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::writeSingleInstanceTransformToCPU(const double* position, const double* orientation, int srcIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::writeSingleInstanceColorToCPU(const float* color, int srcIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::writeSingleInstanceColorToCPU(const double* color, int srcIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::writeSingleInstanceScaleToCPU(const float* scale, int srcIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::writeSingleInstanceScaleToCPU(const double* scale, int srcIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::writeSingleInstanceSpecularColorToCPU(const double* specular, int srcIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::writeSingleInstanceSpecularColorToCPU(const float* specular, int srcIndex)
{
	Assert(false);
}

int FrameworkRenderInterface::getTotalNumInstances() const
{
	Assert(false);
	return 0;
}

void FrameworkRenderInterface::writeTransforms()
{
	Assert(false);
}

void FrameworkRenderInterface::clearZBuffer()
{
	Assert(false);
}

struct GLInstanceRendererInternalData * FrameworkRenderInterface::getInternalData()
{
	return nullptr;
}
