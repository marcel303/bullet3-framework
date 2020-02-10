#include "FrameworkExampleBrowser.h"

#include "LinearMath/btQuickprof.h"

#include "../bullet3/examples/Utils/ChromeTraceUtil.h"

#include "../bullet3/examples/CommonInterfaces/CommonGraphicsAppInterface.h"
#include "../bullet3/examples/CommonInterfaces/CommonParameterInterface.h"
#include "../bullet3/examples/ExampleBrowser/ExampleEntries.h"

#include "LinearMath/btIDebugDraw.h"
#include "LinearMath/btAlignedObjectArray.h"

#include "Bullet3Common/b3AlignedObjectArray.h"
#include "Bullet3Common/b3FileUtils.h"

#include "framework.h"
#include "FrameworkGuiHelper.h"
#include "FrameworkRenderInterface.h"
#include "SimpleFrameworkApp.h"

#define TODO_GUI2 false

static CommonGraphicsApp * s_app = nullptr;
static CommonWindowInterface * s_window = nullptr;
static CommonRenderInterface * s_instancingRenderer = nullptr;
static CommonParameterInterface * s_parameterInterface = nullptr;
static FrameworkGUIHelperInterface * s_guiHelper = nullptr;

static SharedMemoryInterface * sSharedMem = nullptr;

#define DEMO_SELECTION_COMBOBOX 13

const char * startFileName = "0_Bullet3Demo.txt";

static bool gEnableDefaultKeyboardShortcuts = true;
static int sCurrentDemoIndex = -1;
static int sCurrentHightlighted = 0;
static CommonExampleInterface * sCurrentDemo = 0;
static float gFixedTimeStep = 0;
static bool gDisableDemoSelection = false;
static class ExampleEntries* gAllExamples = 0;

static bool visualWireframe = true;
static bool renderVisualGeometry = true;
static bool renderGrid = true;
static bool gEnableRenderLoop = true;

static bool renderGui = true;

//int gDebugDrawFlags = 0; // todo : enable keyboard shortcuts for changing debug draw mode
static int gDebugDrawFlags =
	btIDebugDraw::DBG_DrawWireframe |
	btIDebugDraw::DBG_DrawAabb |
	btIDebugDraw::DBG_DrawContactPoints |
	btIDebugDraw::DBG_DrawFeaturesText |
	btIDebugDraw::DBG_DrawNormals;
	
static bool pauseSimulation = false;
static bool singleStepSimulation = false;
extern bool gDisableDeactivation;

//

static void selectDemo(int demoIndex);
static void deleteDemo();

//

static void selectDemo(int demoIndex)
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

static void deleteDemo()
{
	if (sCurrentDemo)
	{
		sCurrentDemo->exitPhysics();
		s_instancingRenderer->removeAllInstances();
		delete sCurrentDemo;
		sCurrentDemo = 0;
	}
}

//

static b3KeyboardCallback prevKeyboardCallback = nullptr;

void MyKeyboardCallback(int key, int state)
{
	//b3Printf("key=%d, state=%d", key, state);
	
	bool handled = false;
	
	if (renderGui)
	{
	#if TODO_GUI2
		if (gui2 && !handled)
		{
			handled = gui2->keyboardCallback(key, state);
		}
	#endif
	}

	if (!handled && sCurrentDemo)
	{
		handled = sCurrentDemo->keyboardCallback(key, state);
	}

	if (gEnableDefaultKeyboardShortcuts)
	{
		if (key == 's' && state)
		{
			if (gAllExamples->getNumRegisteredExamples() > 0)
			{
				srand(time(nullptr));
				selectDemo(rand() % gAllExamples->getNumRegisteredExamples());
			}
		}
		
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
	}
	
	if (prevKeyboardCallback)
		prevKeyboardCallback(key, state);
}

static b3MouseMoveCallback prevMouseMoveCallback = nullptr;

static void MyMouseMoveCallback(float x, float y)
{
	bool handled = false;
	
	if (sCurrentDemo)
		handled = sCurrentDemo->mouseMoveCallback(x, y);
	
	if (renderGui)
	{
	#if TODO_GUI2
		if (!handled && gui2)
			handled = gui2->mouseMoveCallback(x, y);
	#endif
	}
	
	if (!handled)
	{
		if (prevMouseMoveCallback)
			prevMouseMoveCallback(x, y);
	}
}

static b3MouseButtonCallback prevMouseButtonCallback = nullptr;

static void MyMouseButtonCallback(int button, int state, float x, float y)
{
	bool handled = false;
	
	//try picking first
	if (sCurrentDemo)
		handled = sCurrentDemo->mouseButtonCallback(button, state, x, y);

	if (renderGui)
	{
	#if TODO_GUI2
		if (!handled && gui2)
			handled = gui2->mouseButtonCallback(button, state, x, y);
	#endif
	}
	
	if (!handled)
	{
		if (prevMouseButtonCallback)
			prevMouseButtonCallback(button, state, x, y);
	}
}

//

#include <string.h>

struct FileImporterByExtension
{
	std::string m_extension;
	CommonExampleInterface::CreateFunc* m_createFunc;
};

static btAlignedObjectArray<FileImporterByExtension> gFileImporterByExtension;

void FrameworkExampleBrowser::registerFileImporter(
	const char* extension,
	CommonExampleInterface::CreateFunc* createFunc)
{
	FileImporterByExtension fi;
	fi.m_extension = extension;
	fi.m_createFunc = createFunc;
	gFileImporterByExtension.push_back(fi);
}

static void openFileDemo(const char* filename)
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

//

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
	s_app->m_2dCanvasInterface = nullptr;

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

	prevMouseMoveCallback = s_window->getMouseMoveCallback();
	s_window->setMouseMoveCallback(MyMouseMoveCallback);

	prevMouseButtonCallback = s_window->getMouseButtonCallback();
	s_window->setMouseButtonCallback(MyMouseButtonCallback);
	prevKeyboardCallback = s_window->getKeyboardCallback();
	s_window->setKeyboardCallback(MyKeyboardCallback);

	s_app->m_renderer->getActiveCamera()->setCameraDistance(13);
	s_app->m_renderer->getActiveCamera()->setCameraPitch(0);
	s_app->m_renderer->getActiveCamera()->setCameraTargetPosition(0, 0, 0);

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
	b3ChromeUtilsEnableProfiling();

	if (!gEnableRenderLoop && !singleStepSimulation)
	{
		B3_PROFILE("updateGraphics");
		sCurrentDemo->updateGraphics();
		return;
	}

	B3_PROFILE("FrameworkExampleBrowser::update");
	
	static int frameCount = 0;
	frameCount++;
	
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
			
			DrawGridData dg;
			dg.upAxis = s_app->getUpAxis();
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
			ExampleEntry(1, "TestPendulum", "Simulate a pendulum using btMultiBody with a constant joint torque applied. The same code is also used as a unit test comparing Bullet with the numerical solution of second-order non-linear differential equation stored in pendulum_gold.h", TestPendulumCreateFunc),
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

int main(int argc, char * argv[])
{
	setupPaths(CHIBI_RESOURCE_PATHS);
	
	s_app = new SimpleFrameworkApp("Example Browser", 800, 600);
	
	s_window = s_app->m_window;
	
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
			auto * app = s_app;
			
			if (app->m_window->requestedExit())
				break;
			
			app->m_renderer->init();
			app->m_renderer->updateCamera(app->getUpAxis());

			exampleBrowser->update(framework.timeStep);

			//auto * example = exampleBrowser->getCurrentExample();
			//if (example != nullptr)
			//	example->physicsDebugDraw(~0);

			app->swapBuffer();
		}
	}

	delete exampleBrowser;
	exampleBrowser = nullptr;
	
	delete s_guiHelper;
	s_guiHelper = nullptr;
	
	delete s_app;
	s_app = nullptr;
}
