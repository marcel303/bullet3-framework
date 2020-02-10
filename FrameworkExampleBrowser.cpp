#include "FrameworkExampleBrowser.h"

#include "LinearMath/btQuickprof.h"
#include "../bullet3/examples/CommonInterfaces/CommonRenderInterface.h"

#include "LinearMath/btThreads.h"
#include "Bullet3Common/b3Vector3.h"

#include "assert.h"
#include <stdio.h>

#include "../bullet3/examples/Utils/ChromeTraceUtil.h"

#include "../bullet3/examples/CommonInterfaces/CommonGraphicsAppInterface.h"
#include "../bullet3/examples/CommonInterfaces/Common2dCanvasInterface.h"
#include "../bullet3/examples/CommonInterfaces/CommonExampleInterface.h"
#include "../bullet3/examples/CommonInterfaces/CommonParameterInterface.h"
#include "../bullet3/examples/CommonInterfaces/CommonGUIHelperInterface.h"
#include "../bullet3/examples/ExampleBrowser/ExampleEntries.h"
#include "Bullet3Common/b3FileUtils.h"

#include "LinearMath/btIDebugDraw.h"
#include "LinearMath/btAlignedObjectArray.h"
//quick test for file import, @todo(erwincoumans) make it more general and add other file formats
#include "../bullet3/examples/Importers/ImportURDFDemo/ImportURDFSetup.h"
#include "../bullet3/examples/Importers/ImportBullet/SerializeSetup.h"
#include "Bullet3Common/b3HashMap.h"

#include "framework.h"
#include "FrameworkGuiHelper.h"
#include "FrameworkRenderInterface.h"

static CommonGraphicsApp * s_app = 0;
static CommonRenderInterface * s_instancingRenderer = 0;
static CommonParameterInterface * s_parameterInterface = 0;
static FrameworkGUIHelperInterface * s_guiHelper = 0;

static SharedMemoryInterface * sSharedMem = 0;

#define DEMO_SELECTION_COMBOBOX 13

const char* startFileName = "0_Bullet3Demo.txt";

char staticPngFileName[1024];
static int sCurrentDemoIndex = -1;
static int sCurrentHightlighted = 0;
static CommonExampleInterface* sCurrentDemo = 0;
static b3AlignedObjectArray<const char*> allNames;
static float gFixedTimeStep = 0;
bool gAllowRetina = true;
bool gDisableDemoSelection = false;
int gRenderDevice = -1;
int gWindowBackend = 0;
static class ExampleEntries* gAllExamples = 0;

bool visualWireframe = false;
static bool renderVisualGeometry = true;
static bool renderGrid = true;
static bool gEnableRenderLoop = true;

bool renderGui = true;

//int gDebugDrawFlags = 0; // todo : enable keyboard shortcuts for changing debug draw mode
int gDebugDrawFlags =
	btIDebugDraw::DBG_DrawAabb |
	btIDebugDraw::DBG_DrawContactPoints |
	btIDebugDraw::DBG_DrawFeaturesText |
	btIDebugDraw::DBG_DrawNormals;
	
static bool pauseSimulation = false;
static bool singleStepSimulation = false;
int midiBaseIndex = 176;
extern bool gDisableDeactivation;

int gSharedMemoryKey = -1;

// some quick test variables for the OpenCL examples
int gPreferredOpenCLDeviceIndex = -1;
int gPreferredOpenCLPlatformIndex = -1;
int gGpuArraySizeX = 45;
int gGpuArraySizeY = 55;
int gGpuArraySizeZ = 45;

static Camera3d s_camera;

void deleteDemo()
{
	if (sCurrentDemo)
	{
		sCurrentDemo->exitPhysics();
		s_instancingRenderer->removeAllInstances();
		delete sCurrentDemo;
		sCurrentDemo = 0;
		
		//		CProfileManager::CleanupMemory();
	}
}

#if TODO_KEYBOARD

b3KeyboardCallback prevKeyboardCallback = 0;

void MyKeyboardCallback(int key, int state)
{
	//b3Printf("key=%d, state=%d", key, state);
	bool handled = false;
	if (renderGui)
	{
		if (gui2 && !handled)
		{
			handled = gui2->keyboardCallback(key, state);
		}
	}

	if (!handled && sCurrentDemo)
	{
		handled = sCurrentDemo->keyboardCallback(key, state);
	}

	//checkout: is it desired to ignore keys, if the demo already handles them?
	//if (handled)
	//	return;

	if (gEnableDefaultKeyboardShortcuts)
	{
		if (key == 'a' && state)
		{
			gDebugDrawFlags ^= btIDebugDraw::DBG_DrawAabb;
		}
		if (key == 'c' && state)
		{
			gDebugDrawFlags ^= btIDebugDraw::DBG_DrawContactPoints;
		}
		if (key == 'd' && state)
		{
			gDebugDrawFlags ^= btIDebugDraw::DBG_NoDeactivation;
			gDisableDeactivation = ((gDebugDrawFlags & btIDebugDraw::DBG_NoDeactivation) != 0);
		}
		if (key == 'j' && state)
		{
			gDebugDrawFlags ^= btIDebugDraw::DBG_DrawFrames;
		}

		if (key == 'k' && state)
		{
			gDebugDrawFlags ^= btIDebugDraw::DBG_DrawConstraints;
		}

		if (key == 'l' && state)
		{
			gDebugDrawFlags ^= btIDebugDraw::DBG_DrawConstraintLimits;
		}
		if (key == 'w' && state)
		{
			visualWireframe = !visualWireframe;
			gDebugDrawFlags ^= btIDebugDraw::DBG_DrawWireframe;
		}

		if (key == 'v' && state)
		{
			renderVisualGeometry = !renderVisualGeometry;
		}
		if (key == 'g' && state)
		{
			renderGrid = !renderGrid;
			renderGui = !renderGui;
		}

		if (key == 'i' && state)
		{
			pauseSimulation = !pauseSimulation;
		}
		if (key == 'o' && state)
		{
			singleStepSimulation = true;
		}

		if (key == 'p')
		{
			if (state)
			{
				b3ChromeUtilsStartTimings();
			}
			else
			{
#ifdef _WIN32
				b3ChromeUtilsStopTimingsAndWriteJsonFile("timings");
#else
				b3ChromeUtilsStopTimingsAndWriteJsonFile("/tmp/timings");
#endif
			}
		}

		if (key == 's' && state)
		{
			useShadowMap = !useShadowMap;
		}
		
		if (key == B3G_F1)
		{
			static int count = 0;
			if (state)
			{
				b3Printf("F1 pressed %d", count++);

				if (gPngFileName)
				{
					b3Printf("disable image dump");

					gPngFileName = 0;
				}
				else
				{
					gPngFileName = gAllExamples->getExampleName(sCurrentDemoIndex);
					b3Printf("enable image dump %s", gPngFileName);
				}
			}
			else
			{
				b3Printf("F1 released %d", count++);
			}
		}
	}
	if (prevKeyboardCallback)
		prevKeyboardCallback(key, state);
}

#endif

#if TODO_MOUSE

b3MouseMoveCallback prevMouseMoveCallback = 0;
static void MyMouseMoveCallback(float x, float y)
{
	bool handled = false;
	if (sCurrentDemo)
		handled = sCurrentDemo->mouseMoveCallback(x, y);
	if (renderGui)
	{
		if (!handled && gui2)
			handled = gui2->mouseMoveCallback(x, y);
	}
	if (!handled)
	{
		if (prevMouseMoveCallback)
			prevMouseMoveCallback(x, y);
	}
}

b3MouseButtonCallback prevMouseButtonCallback = 0;

static void MyMouseButtonCallback(int button, int state, float x, float y)
{
	bool handled = false;
	//try picking first
	if (sCurrentDemo)
		handled = sCurrentDemo->mouseButtonCallback(button, state, x, y);

	if (renderGui)
	{
		if (!handled && gui2)
			handled = gui2->mouseButtonCallback(button, state, x, y);
	}
	if (!handled)
	{
		if (prevMouseButtonCallback)
			prevMouseButtonCallback(button, state, x, y);
	}
	//	b3DefaultMouseButtonCallback(button,state,x,y);
}

#endif

#include <string.h>
struct FileImporterByExtension
{
	std::string m_extension;
	CommonExampleInterface::CreateFunc* m_createFunc;
};

static btAlignedObjectArray<FileImporterByExtension> gFileImporterByExtension;

void FrameworkExampleBrowser::registerFileImporter(const char* extension, CommonExampleInterface::CreateFunc* createFunc)
{
	FileImporterByExtension fi;
	fi.m_extension = extension;
	fi.m_createFunc = createFunc;
	gFileImporterByExtension.push_back(fi);
}

#include "../bullet3/examples/SharedMemory/SharedMemoryPublic.h"

#if TODO_VISUALIZER

void OpenGLExampleBrowserVisualizerFlagCallback(int flag, bool enable)
{
	if (flag == COV_ENABLE_Y_AXIS_UP)
	{
		//either Y = up or Z
		int upAxis = enable ? 1 : 2;
		s_app->setUpAxis(upAxis);
	}

	if (flag == COV_ENABLE_RENDERING)
	{
		gEnableRenderLoop = (enable != 0);
	}

	if (flag == COV_ENABLE_SINGLE_STEP_RENDERING)
	{
		if (enable)
		{
			gEnableRenderLoop = false;
			singleStepSimulation = true;
		}
		else
		{
			gEnableRenderLoop = true;
			singleStepSimulation = false;
		}
	}

	if (flag == COV_ENABLE_SHADOWS)
	{
		useShadowMap = enable;
	}
	if (flag == COV_ENABLE_GUI)
	{
		renderGui = enable;
		renderGrid = enable;
	}

	if (flag == COV_ENABLE_KEYBOARD_SHORTCUTS)
	{
		gEnableDefaultKeyboardShortcuts = enable;
	}
	if (flag == COV_ENABLE_MOUSE_PICKING)
	{
		gEnableDefaultMousePicking = enable;
	}

	if (flag == COV_ENABLE_WIREFRAME)
	{
		visualWireframe = enable;
		if (visualWireframe)
		{
			gDebugDrawFlags |= btIDebugDraw::DBG_DrawWireframe;
		}
		else
		{
			gDebugDrawFlags &= ~btIDebugDraw::DBG_DrawWireframe;
		}
	}
}

#endif

void openFileDemo(const char* filename)
{
	deleteDemo();

	s_parameterInterface->removeAllParameters();

	CommonExampleOptions options(s_guiHelper, 1);
	options.m_fileName = filename;
	char fullPath[1024];
	sprintf(fullPath, "%s", filename);
	b3FileUtils::toLower(fullPath);

	for (int i = 0; i < gFileImporterByExtension.size(); i++)
	{
		if (strstr(fullPath, gFileImporterByExtension[i].m_extension.c_str()))
		{
			sCurrentDemo = gFileImporterByExtension[i].m_createFunc(options);
		}
	}

	if (sCurrentDemo)
	{
		sCurrentDemo->initPhysics();
		sCurrentDemo->resetCamera();
	}
}

void selectDemo(int demoIndex)
{
	bool resetCamera = (sCurrentDemoIndex != demoIndex);
	sCurrentDemoIndex = demoIndex;
	sCurrentHightlighted = demoIndex;
	int numDemos = gAllExamples->getNumRegisteredExamples();

	if (demoIndex > numDemos)
	{
		demoIndex = 0;
	}
	deleteDemo();

	CommonExampleInterface::CreateFunc* func = gAllExamples->getExampleCreateFunc(demoIndex);
	if (func)
	{
		if (s_parameterInterface)
		{
			s_parameterInterface->removeAllParameters();
		}
		int option = gAllExamples->getExampleOption(demoIndex);

		CommonExampleOptions options(s_guiHelper, option);
		options.m_sharedMem = sSharedMem;
		sCurrentDemo = (*func)(options);
		if (sCurrentDemo)
		{
			b3Printf("Selected demo: %s", gAllExamples->getExampleName(demoIndex));

			sCurrentDemo->initPhysics();
			if (resetCamera)
			{
				sCurrentDemo->resetCamera();
			}
		}
	}
}

#if TODO_COMBOBOX
void MyComboBoxCallback(int comboId, const char* item)
{
	//printf("comboId = %d, item = %s\n",comboId, item);
	if (comboId == DEMO_SELECTION_COMBOBOX)
	{
		//find selected item
		for (int i = 0; i < allNames.size(); i++)
		{
			if (strcmp(item, allNames[i]) == 0)
			{
				selectDemo(i);
				saveCurrentSettings(sCurrentDemoIndex, startFileName);
				break;
			}
		}
	}
}

#endif

FrameworkExampleBrowser::FrameworkExampleBrowser(ExampleEntries * examples)
{
	gAllExamples = examples;
}

FrameworkExampleBrowser::~FrameworkExampleBrowser()
{
	deleteDemo();

	delete s_parameterInterface;
	s_parameterInterface = 0;
	
	delete s_app->m_2dCanvasInterface;
	s_app->m_2dCanvasInterface = 0;

#ifndef BT_NO_PROFILE
	destroyProfileWindow(m_internalData->m_profWindow);
#endif

	gFileImporterByExtension.clear();
	gAllExamples = 0;
}

#include "../bullet3/examples/ExampleBrowser/EmptyExample.h"

bool FrameworkExampleBrowser::init(int argc, char* argv[])
{
	s_instancingRenderer = s_app->m_renderer;

#if 0
	prevMouseMoveCallback = s_window->getMouseMoveCallback();
	s_window->setMouseMoveCallback(MyMouseMoveCallback);

	prevMouseButtonCallback = s_window->getMouseButtonCallback();
	s_window->setMouseButtonCallback(MyMouseButtonCallback);
	prevKeyboardCallback = s_window->getKeyboardCallback();
	s_window->setKeyboardCallback(MyKeyboardCallback);
#endif

	s_app->m_renderer->getActiveCamera()->setCameraDistance(13);
	s_app->m_renderer->getActiveCamera()->setCameraPitch(0);
	s_app->m_renderer->getActiveCamera()->setCameraTargetPosition(0, 0, 0);

#if TODO_FUNCS
	b3SetCustomWarningMessageFunc(MyGuiPrintf);
	b3SetCustomPrintfFunc(MyGuiPrintf);
	b3SetCustomErrorMessageFunc(MyStatusBarError);
#endif

	// add some demos to the gAllExamples

	int numDemos = gAllExamples->getNumRegisteredExamples();

	if (sCurrentDemo == nullptr)
	{
		int firstAvailableDemoIndex = -1;
		
		for (int d = 0; d < numDemos; d++)
			if (gAllExamples->getExampleCreateFunc(d) != nullptr)
				if (firstAvailableDemoIndex < 0)
					firstAvailableDemoIndex = d;
		
		if (firstAvailableDemoIndex >= 0)
		{
			selectDemo(firstAvailableDemoIndex);
		}
	}

	btAssert(sCurrentDemo != 0);
	
	if (sCurrentDemo == 0)
	{
		printf("Error, no demo/example\n");
		return false;
	}

	return true;
}

CommonExampleInterface* FrameworkExampleBrowser::getCurrentExample()
{
	btAssert(sCurrentDemo);
	return sCurrentDemo;
}

bool FrameworkExampleBrowser::requestedExit()
{
	return framework.quitRequested;
}

void FrameworkExampleBrowser::updateGraphics()
{
	if (sCurrentDemo)
	{
		if (!pauseSimulation || singleStepSimulation)
		{
			//B3_PROFILE("sCurrentDemo->updateGraphics");
			sCurrentDemo->updateGraphics();
		}
	}
}

void FrameworkExampleBrowser::update(float deltaTime)
{
	s_camera.tick(deltaTime, true);
	
	b3ChromeUtilsEnableProfiling();

	if (!gEnableRenderLoop && !singleStepSimulation)
	{
		B3_PROFILE("updateGraphics");
		sCurrentDemo->updateGraphics();
		return;
	}

	B3_PROFILE("FrameworkExampleBrowser::update");
	{
		B3_PROFILE("s_instancingRenderer");
		//s_instancingRenderer->init();
	}
	
	DrawGridData dg;
	dg.upAxis = s_app->getUpAxis();

	{
		BT_PROFILE("Update Camera and Light");

		//s_instancingRenderer->updateCamera(dg.upAxis);
	}

	static int frameCount = 0;
	frameCount++;

	float r, g, b;
	s_app->getBackgroundColor(&r, &g, &b);
	framework.beginDraw(r * 255.f, g * 255.f, b * 255.f, 0);
	projectPerspective3d(90.f, .1f, 1000.f);
	s_camera.pushViewMatrix();
	
	if (0)
	{
		BT_PROFILE("Draw frame counter");
		char bla[1024];
		sprintf(bla, "Frame %d", frameCount);
		s_app->drawText(bla, 10, 10);
	}

	if (sCurrentDemo)
	{
		if (!pauseSimulation || singleStepSimulation)
		{
			//printf("---------------------------------------------------\n");
			//printf("Framecount = %d\n",frameCount);
			B3_PROFILE("sCurrentDemo->stepSimulation");

			if (gFixedTimeStep > 0)
			{
				
				sCurrentDemo->stepSimulation(gFixedTimeStep);
			}
			else
			{
				sCurrentDemo->stepSimulation(deltaTime);  //1./60.f);
			}
		}

		if (renderGrid)
		{
			BT_PROFILE("Draw Grid");
			s_app->drawGrid(dg);
		}
		
		if (renderVisualGeometry && ((gDebugDrawFlags & btIDebugDraw::DBG_DrawWireframe) == 0))
		{
			BT_PROFILE("Render Scene");
			
			pushWireframe(visualWireframe);
			{
				sCurrentDemo->renderScene();
			}
			popWireframe();
		}
		else
		{
			B3_PROFILE("physicsDebugDraw");
			sCurrentDemo->physicsDebugDraw(gDebugDrawFlags);
		}
	}

	static int toggle = 1; // todo : remove static
	
	if (renderGui)
	{
		B3_PROFILE("renderGui");
#ifndef BT_NO_PROFILE

		if (!pauseSimulation || singleStepSimulation)
		{
			if (isProfileWindowVisible(s_profWindow))
			{
				processProfileData(s_profWindow, false);
			}
		}
#endif  //#ifndef BT_NO_PROFILE
	}

	singleStepSimulation = false;

	toggle = 1 - toggle;
	
	{
		BT_PROFILE("Sync Parameters");
		if (s_parameterInterface)
		{
			s_parameterInterface->syncParameters();
		}
	}
	
	s_camera.popViewMatrix();
	framework.endDraw();
}

void FrameworkExampleBrowser::setSharedMemoryInterface(class SharedMemoryInterface* sharedMem)
{
	gDisableDemoSelection = true;
	sSharedMem = sharedMem;
}

//

#include "../bullet3/examples/CommonInterfaces/CommonExampleInterface.h"
#include "../bullet3/examples/MultiBody/Pendulum.h"
#include "../bullet3/examples/VoronoiFracture/VoronoiFractureDemo.h"
#include <vector>

class MyExampleEntries : public ExampleEntries
{
	struct ExampleEntry
	{
		int m_menuLevel = 0;
		const char * m_name = nullptr;
		const char * m_description = nullptr;
		CommonExampleInterface::CreateFunc * m_createFunc = nullptr;
		int m_option = 0;

		ExampleEntry(int menuLevel, const char * name)
			: m_menuLevel(menuLevel)
			, m_name(name)
			, m_description(0)
			, m_createFunc(0)
			, m_option(0)
		{
		}

		ExampleEntry(int menuLevel, const char * name, const char * description, CommonExampleInterface::CreateFunc * createFunc, int option = 0)
			: m_menuLevel(menuLevel)
			, m_name(name)
			, m_description(description)
			, m_createFunc(createFunc)
			, m_option(option)
		{
		}
	};
	
	std::vector<ExampleEntry> exampleEntries;
	
public:
	virtual ~MyExampleEntries() override final
	{
	}

	virtual void initExampleEntries() override final
	{
		static ExampleEntry gDefaultExamples[] =
		{
			//ExampleEntry(1, "TestPendulum", "Simulate a pendulum using btMultiBody with a constant joint torque applied. The same code is also used as a unit test comparing Bullet with the numerical solution of second-order non-linear differential equation stored in pendulum_gold.h", TestPendulumCreateFunc),
			ExampleEntry(1, "Voronoi Fracture", "Automatically create a compound rigid body using voronoi tesselation. Individual parts are modeled as rigid bodies using a btConvexHullShape.",
					 VoronoiFractureCreateFunc)
		};
		
		for (auto & exampleEntry : gDefaultExamples)
			exampleEntries.push_back(exampleEntry);
	}

	virtual void initOpenCLExampleEntries() override final
	{
	}

	virtual int getNumRegisteredExamples() override final
	{
		return exampleEntries.size();
	}

	virtual CommonExampleInterface::CreateFunc * getExampleCreateFunc(int index) override final
	{
		return exampleEntries[index].m_createFunc;
	}

	virtual const char* getExampleName(int index) override final
	{
		return exampleEntries[index].m_name;
	}

	virtual const char* getExampleDescription(int index) override final
	{
		return exampleEntries[index].m_description;
	}

	virtual int getExampleOption(int index) override final
	{
		return exampleEntries[index].m_option;
	}
};

struct MyApp : public CommonGraphicsApp
{
	MyApp()
	{
		m_renderer = new FrameworkRenderInterface();
	}
	
	virtual ~MyApp() override final
	{
	}
	
	virtual int registerCubeShape(float halfExtentsX = 1.f, float halfExtentsY = 1.f, float halfExtentsZ = 1.f, int textureIndex = -1, float textureScaling = 1) override final
	{
		return 0;
	}
	
	virtual int registerGraphicsUnitSphereShape(EnumSphereLevelOfDetail lod, int textureId = -1) override final
	{
		return 0;
	}
	
	virtual void registerGrid(int xres, int yres, float color0[4], float color1[4]) override final
	{
		Assert(false);
	}
	
	virtual void getScreenPixels(unsigned char* rgbaBuffer, int bufferSizeInBytes, float* depthBuffer, int depthBufferSizeInBytes) override final
	{
	}
	
	virtual void setViewport(int width, int height) override final
	{
		Assert(false);
	}

	virtual void drawGrid(DrawGridData data = DrawGridData()) override final
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
	
	virtual void setUpAxis(int axis) override final
	{
		Assert(false);
	}
	
	virtual int getUpAxis() const override final
	{
		return 1;
	}

	virtual void swapBuffer() override final
	{
		Assert(false);
	}
	
	virtual void drawText(const char* txt, int posX, int posY, float size, float colorRGBA[4]) override final
	{
		::setColorf(colorRGBA[0], colorRGBA[1], colorRGBA[2], colorRGBA[3]);
		::drawText(posX, posY, size, +1, -1, "%s", txt);
	}
	
	virtual void drawText3D(const char* txt, float posX, float posZY, float posZ, float size) override final
	{
		Assert(false);
	}
	
	virtual void drawText3D(const char* txt, float position[3], float orientation[4], float color[4], float size, int optionFlag) override final
	{
		Assert(false);
	}

	virtual void drawTexturedRect(float x0, float y0, float x1, float y1, float color[4], float u0, float v0, float u1, float v1, int useRGBA) override final
	{
		Assert(false);
	}
};

int main(int argc, char * argv[])
{
	if (!framework.init(800, 600))
		return -1;
	
	s_app = new MyApp();
	
	s_guiHelper = new FrameworkGUIHelperInterface();
	s_guiHelper->m_renderInterface = s_app->m_renderer;
	s_guiHelper->m_appInterface = s_app;
	
	MyExampleEntries examples;
	examples.initExampleEntries();
	
	auto * exampleBrowser = new FrameworkExampleBrowser(&examples);
	
	const bool init = exampleBrowser->init(argc, argv);
	
#if TODO_FILE_IMPORTERS
	exampleBrowser->registerFileImporter(".urdf", ImportURDFCreateFunc);
	exampleBrowser->registerFileImporter(".sdf", ImportSDFCreateFunc);
	exampleBrowser->registerFileImporter(".obj", ImportObjCreateFunc);
	exampleBrowser->registerFileImporter(".stl", ImportSTLCreateFunc);
	exampleBrowser->registerFileImporter(".bullet", SerializeBulletCreateFunc);
#endif

	if (init)
	{
		for (;;)
		{
			framework.process();
			
			if (framework.quitRequested || exampleBrowser->requestedExit())
				break;
			
			exampleBrowser->update(framework.timeStep);
		}
	}

	delete exampleBrowser;
	exampleBrowser = nullptr;
	
	delete s_guiHelper;
	s_guiHelper = nullptr;
	
	delete s_app;
	s_app = nullptr;
	
	framework.shutdown();
}
