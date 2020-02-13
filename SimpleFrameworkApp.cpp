#include "framework.h"
#include "SimpleFrameworkApp.h"
#include "OpenGLWindow/ShapeData.h"

#include "Bullet3Common/b3Vector3.h"
#include "Bullet3Common/b3Logging.h"
#include "Bullet3Common/b3Quaternion.h"
#include "LinearMath/btAlignedObjectArray.h"

#include "FrameworkCanvasInterface.h" // todo : rename to 2dCanvasInterface.h
#include "FrameworkParameterInterface.h"
#include "FrameworkRenderInterface.h"
#include "FrameworkWindow.h"

#define TODO 0

struct SimpleInternalData
{
	int m_upAxis;  //y=1 or z=2 is supported
	int m_customViewPortWidth;
	int m_customViewPortHeight;
	
	SimpleInternalData()
		: m_upAxis(1)
		, m_customViewPortWidth(-1)
		, m_customViewPortHeight(-1)
	{
	}
};

static SimpleFrameworkApp * gApp = nullptr;

static void SimpleResizeCallback(float widthf, float heightf)
{
	int width = (int)widthf;
	int height = (int)heightf;
	
	if (gApp && gApp->m_renderer)
		gApp->m_renderer->resize(width, height);
}

static void SimpleKeyboardCallback(int key, int state)
{
	if (key == SDLK_ESCAPE && gApp && gApp->m_window)
	{
		gApp->m_window->setRequestExit();
	}
	else
	{
		//gApp->defaultKeyboardCallback(key,state);
	}
}

static void SimpleMouseButtonCallback(int button, int state, float x, float y)
{
	gApp->defaultMouseButtonCallback(button, state, x, y);
}

static void SimpleMouseMoveCallback(float x, float y)
{
	gApp->defaultMouseMoveCallback(x, y);
}

static void SimpleWheelCallback(float deltax, float deltay)
{
	gApp->defaultWheelCallback(deltax, deltay);
}

SimpleFrameworkApp::SimpleFrameworkApp(
	const char * title,
	int width,
	int height,
	bool allowRetina,
	int windowType,
	int renderDevice,
	int maxNumObjectCapacity,
	int maxShapeCapacityInBytes)
{
	gApp = this;
	
	m_data = new SimpleInternalData();

	m_window = new FrameworkWindow();

	m_window->setAllowRetina(allowRetina);

	b3gWindowConstructionInfo ci;
	ci.m_title = title;
	ci.m_width = width;
	ci.m_height = height;
	ci.m_renderDevice = renderDevice;
	m_window->createWindow(ci);

	m_window->setWindowTitle(title);

	static_cast<FrameworkWindow*>(m_window)->setBackgroundColor(m_backgroundColorRGB);

	m_window->startRendering();
	width = m_window->getWidth();
	height = m_window->getHeight();
	
	m_renderer = new FrameworkRenderInterface();

	m_parameterInterface = new FrameworkParameterInterface();
	
	m_2dCanvasInterface = new FrameworkCanvasInterface();

	m_window->setResizeCallback(SimpleResizeCallback);

	m_window->setMouseMoveCallback(SimpleMouseMoveCallback);
	m_window->setMouseButtonCallback(SimpleMouseButtonCallback);
	m_window->setKeyboardCallback(SimpleKeyboardCallback);
	m_window->setWheelCallback(SimpleWheelCallback);

	m_wheelMultiplier = .1f;
}

// todo : optimize text drawing. currently it does too many state switches

static void beginTextDrawing()
{
	gxMatrixMode(GX_PROJECTION);
	gxPushMatrix();
	gxMatrixMode(GX_MODELVIEW);
	gxPushMatrix();
	
	//
	
	projectScreen2d();
	pushDepthTest(false, DEPTH_LESS);
	pushBlend(BLEND_ALPHA);
	
	setFont("calibri.ttf");
}

static void endTextDrawing()
{
	popDepthTest();
	popBlend();

	setTransform(TRANSFORM_3D);
	
	//
	
	gxMatrixMode(GX_PROJECTION);
	gxPopMatrix();
	gxMatrixMode(GX_MODELVIEW);
	gxPopMatrix();
}

void SimpleFrameworkApp::drawText3D(const char* txt, float position[3], float orientation[4], float color[4], float size, int optionFlag)
{
	float w;
	const Vec2 p = transformToScreen(Vec3(position[0], position[1], position[2]), w);
	
	if (w > 0.f)
	{
		beginTextDrawing();
		{
			::setColorf(color[0], color[1], color[2], color[3]);
			::drawText(p[0], p[1], size * 12.f, 0, 0, "%s", txt);
		}
		endTextDrawing();
	}
}

void SimpleFrameworkApp::drawText3D(const char* txt, float worldPosX, float worldPosY, float worldPosZ, float size1)
{
	float position[3] = {worldPosX, worldPosY, worldPosZ};
	float orientation[4] = {0, 0, 0, 1};
	float color[4] = {0, 0, 0, 1};
	int optionFlags = CommonGraphicsApp::eDrawText3D_OrtogonalFaceCamera;
	drawText3D(txt, position, orientation, color, size1, optionFlags);
}

void SimpleFrameworkApp::drawText(const char* txt, int posX, int posY, float size, float colorRGBA[4])
{
	beginTextDrawing();
	{
		::setColorf(colorRGBA[0], colorRGBA[1], colorRGBA[2], colorRGBA[3]);
		::drawText(posX, posY, size, +1, -1, "%s", txt);
	}
	endTextDrawing();
}

void SimpleFrameworkApp::drawTexturedRect(float x0, float y0, float x1, float y1, float color[4], float u0, float v0, float u1, float v1, int useRGBA)
{
	beginTextDrawing();
	{
	// todo : use custom uv's
	// todo : honor useRGBA
		Assert(false);
		
		::setColorf(color[0], color[1], color[2], color[3]);
		drawRect(x0, y0, x1, y1);
	}
	endTextDrawing();
}

int SimpleFrameworkApp::registerCubeShape(float halfExtentsX, float halfExtentsY, float halfExtentsZ, int textureIndex, float textureScaling)
{
	const int vertexStrideInBytes = 9 * sizeof(float);
	
	const int numVertices = sizeof(cube_vertices_textured) / vertexStrideInBytes;
	
	btAlignedObjectArray<float> vertices;
	vertices.resize(numVertices * 9);
	for (int i = 0; i < numVertices * 9; ++i)
		vertices[i] = cube_vertices_textured[i];
	
	for (int i = 0; i < numVertices; ++i)
	{
		vertices[i * 9 + 0] *= halfExtentsX;
		vertices[i * 9 + 1] *= halfExtentsY;
		vertices[i * 9 + 2] *= halfExtentsZ;
	}
	
	return m_renderer->registerShape(
		&vertices[0],
		numVertices,
		cube_indices,
		sizeof(cube_indices) / sizeof(cube_indices[0]),
		B3_GL_TRIANGLES,
		textureIndex);
}

void SimpleFrameworkApp::registerGrid(int cells_x, int cells_z, float color0[4], float color1[4])
{
	const double halfHeight = 0.1;
	
	b3Vector3 cubeExtents = b3MakeVector3(0.5, 0.5, 0.5);
	cubeExtents[m_data->m_upAxis] = halfHeight;
	
	const int cubeId = registerCubeShape(cubeExtents[0], cubeExtents[1], cubeExtents[2]);
	
	const b3Quaternion orn(0, 0, 0, 1);
	const b3Vector3 center = b3MakeVector3(0, 0, 0, 1);
	const b3Vector3 scaling = b3MakeVector3(1, 1, 1, 1);

	for (int i = 0; i < cells_x; i++)
	{
		for (int j = 0; j < cells_z; j++)
		{
			float * color = nullptr;
			
			if ((i + j) % 2 == 0)
				color = (float*)color0;
			else
				color = (float*)color1;
			
			const b3Vector3 center = m_data->m_upAxis == 1
				? b3MakeVector3((i + 0.5f) - cells_x * 0.5f, -halfHeight, (j + 0.5f) - cells_z * 0.5f)
				: b3MakeVector3((i + 0.5f) - cells_x * 0.5f, (j + 0.5f) - cells_z * 0.5f, -halfHeight);
			
			m_renderer->registerGraphicsInstance(cubeId, center, orn, color, scaling);
		}
	}
}

int SimpleFrameworkApp::registerGraphicsUnitSphereShape(EnumSphereLevelOfDetail lod, int textureId)
{
	const int vertexStrideInBytes = 9 * sizeof(float);

	return m_renderer->registerShape(
		textured_detailed_sphere_vertices,
		sizeof(textured_detailed_sphere_vertices) / vertexStrideInBytes,
		textured_detailed_sphere_indices,
		sizeof(textured_detailed_sphere_indices) / sizeof(textured_detailed_sphere_indices[0]),
		B3_GL_TRIANGLES,
		textureId);
}

void SimpleFrameworkApp::drawGrid(DrawGridData data)
{
    gxPushMatrix();
    {
        Vec3 offset;
        offset[data.upAxis] = data.upOffset;
        gxTranslatef(offset[0], offset[1], offset[2]);
        gxScalef(data.gridSize, data.gridSize, data.gridSize);
        
        int axis1 = (data.upAxis + 1) % 3;
        int axis2 = (data.upAxis + 2) % 3;
        ::setColorf(data.gridColor[0], data.gridColor[1], data.gridColor[2], data.gridColor[3]);
        ::drawGrid3dLine(20, 20, axis1, axis2);
    }
    gxPopMatrix();
}

void SimpleFrameworkApp::setBackgroundColor(float red, float green, float blue)
{
	CommonGraphicsApp::setBackgroundColor(red, green, blue);
	
	static_cast<FrameworkWindow*>(m_window)->setBackgroundColor(m_backgroundColorRGB);
}

SimpleFrameworkApp::~SimpleFrameworkApp()
{
	delete m_2dCanvasInterface;
	m_2dCanvasInterface = nullptr;

	delete m_parameterInterface;
	m_parameterInterface = nullptr;
	
	delete m_renderer;
	m_renderer = nullptr;

	m_window->closeWindow();

	delete m_window;
	m_window = nullptr;
	
	delete m_data;
	m_data = nullptr;
}

void SimpleFrameworkApp::setViewport(int width, int height)
{
	m_data->m_customViewPortWidth = width;
	m_data->m_customViewPortHeight = height;
}

void SimpleFrameworkApp::getScreenPixels(unsigned char* rgbaBuffer, int bufferSizeInBytes, float* depthBuffer, int depthBufferSizeInBytes)
{
}

void SimpleFrameworkApp::swapBuffer()
{
	m_window->endRendering();
	
	m_window->runMainLoop();
	
	m_window->startRendering();
}

void SimpleFrameworkApp::setUpAxis(int axis)
{
	b3Assert((axis == 1) || (axis == 2));  //only Y or Z is supported at the moment
	m_data->m_upAxis = axis;
}

int SimpleFrameworkApp::getUpAxis() const
{
	return m_data->m_upAxis;
}
