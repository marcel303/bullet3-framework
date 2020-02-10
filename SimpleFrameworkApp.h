#pragma once

#include "CommonInterfaces/CommonWindowInterface.h"
#include "CommonInterfaces/CommonGraphicsAppInterface.h"

class FrameworkRenderInterface;

struct SimpleFrameworkApp : public CommonGraphicsApp
{
	struct SimpleInternalData * m_data;

	virtual void setBackgroundColor(float red, float green, float blue) override;

	SimpleFrameworkApp(
		const char * title,
		int width,
		int height,
		bool allowRetina = true,
		int windowType = 0,
		int renderDevice = -1,
		int maxNumObjectCapacity = 128 * 1024,
		int maxShapeCapacityInBytes = 128 * 1024 * 1024);

	virtual ~SimpleFrameworkApp() override;

	virtual int registerCubeShape(float halfExtentsX = 1.f, float halfExtentsY = 1.f, float halfExtentsZ = 1.f, int textureIndex = -1, float textureScaling = 1) override;
	virtual int registerGraphicsUnitSphereShape(EnumSphereLevelOfDetail lod, int textureId = -1) override;
	virtual void registerGrid(int xres, int yres, float color0[4], float color1[4]) override;
	
	virtual void getScreenPixels(unsigned char* rgbaBuffer, int bufferSizeInBytes, float* depthBuffer, int depthBufferSizeInBytes) override;
	virtual void setViewport(int width, int height) override;

	virtual void drawGrid(DrawGridData data = DrawGridData()) override;
	
	virtual void setUpAxis(int axis) override;
	virtual int getUpAxis() const override;

	virtual void swapBuffer() override;
	virtual void drawText(const char* txt, int posX, int posY, float size, float colorRGBA[4]) override;
	virtual void drawText3D(const char* txt, float posX, float posZY, float posZ, float size) override;
	virtual void drawText3D(const char* txt, float position[3], float orientation[4], float color[4], float size, int optionFlag) override;

	virtual void drawTexturedRect(float x0, float y0, float x1, float y1, float color[4], float u0, float v0, float u1, float v1, int useRGBA) override;
};
