#include "framework.h"
#include "SimpleFrameworkApp.h"
#include "OpenGLWindow/ShapeData.h"

#include "Bullet3Common/b3Vector3.h"
#include "Bullet3Common/b3Logging.h"

#include "Bullet3Common/b3Quaternion.h"
#include "FrameworkCanvasInterface.h" // todo : rename to 2dCanvasInterface.h
#include "FrameworkParameterInterface.h"
#include "FrameworkRenderInterface.h"
#include "FrameworkWindow.h"

#define TODO 0

struct SimpleInternalData
{
	//RenderCallbacks * m_renderCallbacks = nullptr;
	//RenderCallbacks * m_renderCallbacks2 = nullptr;

	void * m_userPointer = nullptr;
	int m_upAxis;  //y=1 or z=2 is supported
	int m_customViewPortWidth;
	int m_customViewPortHeight;
	
	SimpleInternalData()
		//: m_renderCallbacks(0)
		//, m_renderCallbacks2(0)
		: m_userPointer(0)
		, m_upAxis(1)
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

#if false

struct MyRenderCallbacks : public RenderCallbacks
{
	b3AlignedObjectArray<unsigned char> m_rgbaTexture;
	float m_color[4];
	float m_worldPosition[3];
	float m_worldOrientation[4];

	MyRenderCallbacks()
	{
		for (int i = 0; i < 4; i++)
		{
			m_color[i] = 1;
			m_worldOrientation[i] = 0;
		}
		
		m_worldPosition[0] = 0;
		m_worldPosition[1] = 0;
		m_worldPosition[2] = 0;

		m_worldOrientation[0] = 0;
		m_worldOrientation[1] = 0;
		m_worldOrientation[2] = 0;
		m_worldOrientation[3] = 1;
	}
	
	virtual ~MyRenderCallbacks() override
	{
		m_rgbaTexture.clear();
	}

	virtual void setWorldPosition(float pos[3]) override
	{
		for (int i = 0; i < 3; i++)
		{
			m_worldPosition[i] = pos[i];
		}
	}

	virtual void setWorldOrientation(float orn[4]) override
	{
		for (int i = 0; i < 4; i++)
		{
			m_worldOrientation[i] = orn[i];
		}
	}

	virtual void setColorRGBA(float color[4]) override
	{
		for (int i = 0; i < 4; i++)
		{
			m_color[i] = color[i];
		}
	}
	
	virtual void updateTexture(sth_texture* texture, sth_glyph* glyph, int textureWidth, int textureHeight) override
	{
		if (glyph)
		{
			m_rgbaTexture.resize(textureWidth * textureHeight * 3);
			
			for (int i = 0; i < textureWidth * textureHeight; i++)
			{
				m_rgbaTexture[i * 3 + 0] = texture->m_texels[i];
				m_rgbaTexture[i * 3 + 1] = texture->m_texels[i];
				m_rgbaTexture[i * 3 + 2] = texture->m_texels[i];
			}
			
			bool flipPixelsY = false;
			
			m_instancingRenderer->updateTexture(m_textureIndex, &m_rgbaTexture[0], flipPixelsY);
		}
		else
		{
			if (textureWidth && textureHeight)
			{
				texture->m_texels = (unsigned char*)malloc(textureWidth * textureHeight);
				memset(texture->m_texels, 0, textureWidth * textureHeight);
				if (m_textureIndex < 0)
				{
					m_rgbaTexture.resize(textureWidth * textureHeight * 3);
					bool flipPixelsY = false;
					m_textureIndex = m_instancingRenderer->registerTexture(&m_rgbaTexture[0], textureWidth, textureHeight, flipPixelsY);

					int strideInBytes = 9 * sizeof(float);
					int numVertices = sizeof(cube_vertices_textured) / strideInBytes;
					int numIndices = sizeof(cube_indices) / sizeof(int);

					float halfExtentsX = 1;
					float halfExtentsY = 1;
					float halfExtentsZ = 1;
					float textureScaling = 4;

					b3AlignedObjectArray<GfxVertexFormat1> verts;
					verts.resize(numVertices);
					for (int i = 0; i < numVertices; i++)
					{
						verts[i].x = halfExtentsX * cube_vertices_textured[i * 9];
						verts[i].y = halfExtentsY * cube_vertices_textured[i * 9 + 1];
						verts[i].z = halfExtentsZ * cube_vertices_textured[i * 9 + 2];
						verts[i].w = cube_vertices_textured[i * 9 + 3];
						verts[i].nx = cube_vertices_textured[i * 9 + 4];
						verts[i].ny = cube_vertices_textured[i * 9 + 5];
						verts[i].nz = cube_vertices_textured[i * 9 + 6];
						verts[i].u = cube_vertices_textured[i * 9 + 7] * textureScaling;
						verts[i].v = cube_vertices_textured[i * 9 + 8] * textureScaling;
					}

					int shapeId = m_instancingRenderer->registerShape(&verts[0].x, numVertices, cube_indices, numIndices, B3_GL_TRIANGLES, m_textureIndex);
					b3Vector3 pos = b3MakeVector3(0, 0, 0);
					b3Quaternion orn(0, 0, 0, 1);
					b3Vector4 color = b3MakeVector4(1, 1, 1, 1);
					b3Vector3 scaling = b3MakeVector3(.1, .1, .1);
					//m_instancingRenderer->registerGraphicsInstance(shapeId, pos, orn, color, scaling);
					m_instancingRenderer->writeTransforms();
				}
				else
				{
					b3Assert(0);
				}
			}
			else
			{
				delete texture->m_texels;
				texture->m_texels = 0;
				//there is no m_instancingRenderer->freeTexture (yet), all textures are released at reset/deletion of the renderer
			}
		}
	}
	
	virtual void render(sth_texture* texture)
	{
		int index = 0;

		float width = 1;
		b3AlignedObjectArray<unsigned int> indices;
		indices.resize(texture->nverts);
		for (int i = 0; i < indices.size(); i++)
		{
			indices[i] = i;
		}

		m_instancingRenderer->drawTexturedTriangleMesh(m_worldPosition, m_worldOrientation, &texture->newverts[0].position.p[0], texture->nverts, &indices[0], indices.size(), m_color, m_textureIndex);
	}
};

#endif

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

	{
	#if TODO
		m_data->m_renderCallbacks = new OpenGL2RenderCallbacks(m_primRenderer);
		m_data->m_renderCallbacks2 = new MyRenderCallbacks(m_instancingRenderer);
	#endif
	}
	
	m_wheelMultiplier = .1f;
}

void SimpleFrameworkApp::drawText3D(const char* txt, float position[3], float orientation[4], float color[4], float size, int optionFlag)
{
	float w;
	const Vec2 p = transformToScreen(Vec3(position[0], position[1], position[2]), w);
	
	if (w > 0.f)
	{
		gxMatrixMode(GX_PROJECTION);
		gxPushMatrix();
		gxMatrixMode(GX_MODELVIEW);
		gxPushMatrix();
		{
			projectScreen2d();
			pushDepthTest(false, DEPTH_LESS);
			pushBlend(BLEND_ALPHA);
			
			setFont("calibri.ttf");
			gxColor4fv(color);
			::drawText(p[0], p[1], size * 12.f, 0, 0, "%s", txt);
			
			popDepthTest();
			popBlend();
			
			setTransform(TRANSFORM_3D);
		}
		gxMatrixMode(GX_PROJECTION);
		gxPopMatrix();
		gxMatrixMode(GX_MODELVIEW);
		gxPopMatrix();
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
	::setColorf(colorRGBA[0], colorRGBA[1], colorRGBA[2], colorRGBA[3]);
	::drawText(posX, posY, size, +1, -1, "%s", txt);
}

void SimpleFrameworkApp::drawTexturedRect(float x0, float y0, float x1, float y1, float color[4], float u0, float v0, float u1, float v1, int useRGBA)
{
	Assert(false); // todo
}

int SimpleFrameworkApp::registerCubeShape(float halfExtentsX, float halfExtentsY, float halfExtentsZ, int textureIndex, float textureScaling)
{
	return 0;
}

void SimpleFrameworkApp::registerGrid(int cells_x, int cells_z, float color0[4], float color1[4])
{
}

int SimpleFrameworkApp::registerGraphicsUnitSphereShape(EnumSphereLevelOfDetail lod, int textureId)
{
	return 0;
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
/*
	delete m_data->m_renderCallbacks;
	m_data->m_renderCallbacks = nullptr;

	delete m_data->m_renderCallbacks2;
	m_data->m_renderCallbacks2 = nullptr;
*/

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
