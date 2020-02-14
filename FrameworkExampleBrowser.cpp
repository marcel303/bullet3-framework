#include "FrameworkExampleBrowser.h"

#include "CommonInterfaces/CommonParameterInterface.h"
#include "ExampleBrowser/ExampleEntries.h"
#include "LinearMath/btQuickprof.h"
#include "Utils/ChromeTraceUtil.h"

#include "LinearMath/btIDebugDraw.h"
#include "LinearMath/btAlignedObjectArray.h"

#include "Bullet3Common/b3FileUtils.h"

#include "framework.h"
#include "FrameworkCanvasInterface.h"
#include "FrameworkDebugDrawer.h"
#include "FrameworkGuiHelper.h"
#include "FrameworkParameterInterface.h"
#include "FrameworkRenderInterface.h"
#include "SimpleFrameworkApp.h"

#include "Importers/ImportURDFDemo/ImportURDFSetup.h"
#include "Importers/ImportSDFDemo/ImportSDFSetup.h"
#include "Importers/ImportObjDemo/ImportObjExample.h"
#include "Importers/ImportSTLDemo/ImportSTLSetup.h"
#include "Importers/ImportBullet/SerializeSetup.h"

#include "imgui-framework.h"

static CommonGraphicsApp * s_app = nullptr;
static CommonWindowInterface * s_window = nullptr;
static FrameworkGUIHelperInterface * s_guiHelper = nullptr;

static CommonExampleInterface * sCurrentDemo = nullptr;
static float gFixedTimeStep = 0.f;
static class ExampleEntries * gAllExamples = nullptr;

static bool inputIsCaptured = false;
static bool renderVisualGeometry = true;
static bool renderGrid = true;
static bool gEnableRenderLoop = true;

//int gDebugDrawFlags = 0; // todo : enable keyboard shortcuts for changing debug draw mode
static int gDebugDrawFlags =
	0*btIDebugDraw::DBG_DrawWireframe |
	0*btIDebugDraw::DBG_DrawAabb |
	1*btIDebugDraw::DBG_DrawContactPoints |
	0*btIDebugDraw::DBG_DrawFeaturesText |
	1*btIDebugDraw::DBG_DrawNormals;
	
static bool pauseSimulation = false;
static bool singleStepSimulation = false;

//

int gSharedMemoryKey = -1;

//

static void selectDemo(int demoIndex);
static void deleteDemo();

//

static void selectDemo(int demoIndex)
{
	const int numDemos = gAllExamples->getNumRegisteredExamples();

	if (demoIndex > numDemos)
	{
		demoIndex = 0;
	}
	
	deleteDemo();

	CommonExampleInterface::CreateFunc * func = gAllExamples->getExampleCreateFunc(demoIndex);
	
	if (func != nullptr)
	{
		if (s_app->m_parameterInterface != nullptr)
		{
			s_app->m_parameterInterface->removeAllParameters();
		}
		
		const int option = gAllExamples->getExampleOption(demoIndex);

		CommonExampleOptions options(s_guiHelper, option);
		sCurrentDemo = func(options);
		
		if (sCurrentDemo != nullptr)
		{
			logDebug("Selected demo: %s", gAllExamples->getExampleName(demoIndex));

			sCurrentDemo->initPhysics();
			sCurrentDemo->resetCamera();
			
			const char * name = gAllExamples->getExampleName(demoIndex);
			s_window->setWindowTitle(name);
		}
	}
}

static void deleteDemo()
{
	if (sCurrentDemo != nullptr)
	{
		sCurrentDemo->exitPhysics();
		
		s_app->m_renderer->removeAllInstances();
		
		delete sCurrentDemo;
		sCurrentDemo = nullptr;
	}
}

//

static b3KeyboardCallback prevKeyboardCallback = nullptr;
static b3MouseMoveCallback prevMouseMoveCallback = nullptr;
static b3MouseButtonCallback prevMouseButtonCallback = nullptr;

void MyKeyboardCallback(int key, int state)
{
	//b3Printf("key=%d, state=%d", key, state);
	
	if (inputIsCaptured == false && sCurrentDemo != nullptr)
	{
		inputIsCaptured = sCurrentDemo->keyboardCallback(key, state);
	}

	if (inputIsCaptured == false)
	{
		if (key == 's' && state)
		{
			if (gAllExamples->getNumRegisteredExamples() > 0)
			{
				// note : some examples explicitly call srand, which would our rand() call below deterministic
				
				static int s = 0;
				srand(s++);
				
				int index;
				for (;;)
				{
					index = rand() % gAllExamples->getNumRegisteredExamples();
					if (gAllExamples->getExampleCreateFunc(index) != nullptr)
						break;
				}
				selectDemo(index);
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
			gDebugDrawFlags ^= btIDebugDraw::DBG_DrawWireframe;
		}

		if (key == 'v' && state)
		{
			renderVisualGeometry = !renderVisualGeometry;
		}
		if (key == 'g' && state)
		{
			renderGrid = !renderGrid;
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
	
	if (inputIsCaptured == false && prevKeyboardCallback != nullptr)
		prevKeyboardCallback(key, state);
}

static void MyMouseMoveCallback(float x, float y)
{
	if (inputIsCaptured == false && sCurrentDemo != nullptr)
		inputIsCaptured = sCurrentDemo->mouseMoveCallback(x, y);
	
	if (inputIsCaptured == false && prevMouseMoveCallback != nullptr)
		prevMouseMoveCallback(x, y);
}

static void MyMouseButtonCallback(int button, int state, float x, float y)
{
	if (inputIsCaptured == false && sCurrentDemo != nullptr)
		inputIsCaptured = sCurrentDemo->mouseButtonCallback(button, state, x, y);

	if (inputIsCaptured == false && prevMouseButtonCallback != nullptr)
		prevMouseButtonCallback(button, state, x, y);
}

//

#include <string>
#include <vector>

struct FileImporterByExtension
{
	std::string m_extension;
	CommonExampleInterface::CreateFunc* m_createFunc;
};

static std::vector<FileImporterByExtension> gFileImporterByExtension;

void FrameworkExampleBrowser::registerFileImporter(
	const char * extension,
	CommonExampleInterface::CreateFunc * createFunc)
{
	FileImporterByExtension fi;
	fi.m_extension = extension;
	fi.m_createFunc = createFunc;
	gFileImporterByExtension.push_back(fi);
}

static void openFileDemo(const char * filename)
{
	deleteDemo();

	s_app->m_parameterInterface->removeAllParameters();

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

	if (sCurrentDemo != nullptr)
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

	gFileImporterByExtension.clear();
	gAllExamples = nullptr;
}

bool FrameworkExampleBrowser::init(int argc, char* argv[])
{
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

	const int numDemos = gAllExamples->getNumRegisteredExamples();

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

	btAssert(sCurrentDemo != nullptr);
	
	if (sCurrentDemo == nullptr)
	{
		logError("no demo/example");
		return false;
	}

	return true;
}

CommonExampleInterface* FrameworkExampleBrowser::getCurrentExample()
{
	return sCurrentDemo;
}

bool FrameworkExampleBrowser::requestedExit()
{
	return framework.quitRequested;
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

	if (sCurrentDemo != nullptr)
	{
		if (!pauseSimulation || singleStepSimulation)
		{
			//printf("---------------------------------------------------\n");
			//printf("Framecount = %d\n",frameCount);
			B3_PROFILE("sCurrentDemo->stepSimulation");

			if (gFixedTimeStep > 0.f)
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
		
		if (renderVisualGeometry)
		{
			const bool wireframe = (gDebugDrawFlags & btIDebugDraw::DBG_DrawWireframe) != 0;
			
			if (wireframe)
			{
				B3_PROFILE("physicsDebugDraw");
				sCurrentDemo->physicsDebugDraw(gDebugDrawFlags);
				s_guiHelper->m_debugDraw->flushLines();
			}
			else
			{
				BT_PROFILE("Render Scene");
				sCurrentDemo->renderScene();
				s_guiHelper->m_debugDraw->flushLines();
			}
		}
	}

	singleStepSimulation = false;
	
	{
		BT_PROFILE("Sync Parameters");
		if (s_app->m_parameterInterface != nullptr)
		{
			s_app->m_parameterInterface->syncParameters();
		}
	}
}

//

static void doParameterInterface(FrameworkParameterInterface * paramInterface)
{
	for (auto & param : paramInterface->m_params)
	{
		switch (param.type)
		{
		case FrameworkParameterInterface::kParamType_Undefined:
			break;
			
		case FrameworkParameterInterface::kParamType_Button:
			if (param.button.m_isTrigger && false)
			{
				if (ImGui::Button(param.button.m_name))
				{
					if (param.button.m_callback != nullptr)
						param.button.m_callback(param.button.m_buttonId, 1, param.button.m_userPointer);
				}
			}
			else
			{
				bool value = param.value != 0.f;
				
				if (ImGui::Selectable(param.button.m_name, &value))
				{
					param.value = value ? 1.f : 0.f;
					if (param.button.m_callback != nullptr)
						param.button.m_callback(param.button.m_buttonId, param.button.m_initialState, param.button.m_userPointer);
				}
			}
			break;
		case FrameworkParameterInterface::kParamType_Slider:
			{
				if (param.slider.m_clampToIntegers)
				{
					int value = *param.slider.m_paramValuePointer;
					if (ImGui::SliderInt(param.slider.m_name, &value, param.slider.m_minVal, param.slider.m_maxVal))
					{
						param.value = value;
						if (param.slider.m_callback != nullptr)
							param.slider.m_callback(value, param.slider.m_userPointer);
					}
				}
				else
				{
					float value = *param.slider.m_paramValuePointer;
					if (ImGui::SliderFloat(param.slider.m_name, &value, param.slider.m_minVal, param.slider.m_maxVal))
					{
						param.value = value;
						if (param.slider.m_callback != nullptr)
							param.slider.m_callback(value, param.slider.m_userPointer);
					}
				}
			}
			break;
		case FrameworkParameterInterface::kParamType_ComboBox:
			{
				ImGui::PushID(&param);
				int item = (int)param.value;
				if (ImGui::Combo("Combo", &item, param.comboBox.m_items, param.comboBox.m_numItems))
				{
					if (param.comboBox.m_callback != nullptr)
					{
						param.comboBox.m_callback(
							param.comboBox.m_comboboxId,
							param.comboBox.m_items[(int)param.value],
							param.comboBox.m_userPointer);
					}
				}
				ImGui::PopID();
			}
			break;
		}
	}
}

static void doImGui(FrameworkImGuiContext & guiContext)
{
	guiContext.processBegin(framework.timeStep, s_app->m_window->getWidth(), s_app->m_window->getHeight(), inputIsCaptured);
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Example"))
			{
				bool isInsideSubMenu = false;
				bool submenuIsVisible = false;
				
				for (int i = 0; i < gAllExamples->getNumRegisteredExamples(); ++i)
				{
					const char * name = gAllExamples->getExampleName(i);
					const auto & createFunc = gAllExamples->getExampleCreateFunc(i);
					
					if (createFunc != nullptr)
					{
						if (isInsideSubMenu == false || submenuIsVisible)
						{
							if (ImGui::MenuItem(name))
								selectDemo(i);
						}
					}
					else
					{
						if (isInsideSubMenu)
						{
							if (submenuIsVisible)
								ImGui::EndMenu();
							isInsideSubMenu = false;
						}
						
						submenuIsVisible = ImGui::BeginMenu(name);
						isInsideSubMenu = true;
					}
				}
				
				if (isInsideSubMenu && submenuIsVisible)
					ImGui::EndMenu();
				
				ImGui::EndMenu();
			}
			
			auto * paramInterface = static_cast<FrameworkParameterInterface*>(s_app->m_parameterInterface);
			
			if (paramInterface->m_params.empty() == false)
			{
				if (ImGui::BeginMenu("Parameters"))
				{
					doParameterInterface(paramInterface);
					
					ImGui::EndMenu();
				}
			}
		}
		ImGui::EndMainMenuBar();
		
		int viewSx;
		int viewSy;
		framework.getCurrentViewportSize(viewSx, viewSy);
		
		//ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
		ImGui::SetNextWindowPos(ImVec2(10, viewSy*3/5), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(0, viewSy*1/3), ImGuiCond_Once);
		ImGui::SetNextWindowBgAlpha(.5f);
		if (ImGui::Begin("Example Browser"))
		{
			static char filter[32];
			ImGui::InputText("Filter", filter, sizeof(filter));
			
			for (int i = 0; i < gAllExamples->getNumRegisteredExamples(); ++i)
			{
				const char * name = gAllExamples->getExampleName(i);
				const auto & createFunc = gAllExamples->getExampleCreateFunc(i);
				
				if (filter[0] != 0 && strcasestr(name, filter) == nullptr)
					continue;
				
				if (createFunc != nullptr)
				{
					ImGui::Indent();
					if (ImGui::Selectable(name))
					//if (ImGui::Button(name))
						selectDemo(i);
					ImGui::Unindent();
				}
				else
				{
					ImGui::Text("%s", name);
				}
			}
		}
		ImGui::End();
		
		auto * canvasInterface = static_cast<FrameworkCanvasInterface*>(s_app->m_2dCanvasInterface);
		
		for (auto & i : canvasInterface->m_canvases)
		{
			auto * canvas = i.second;
			
			if (canvas->textureId == 0)
				continue;
			
			ImGui::PushID(canvas);
			ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_Once);
			if (ImGui::Begin("Canvas", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Image(canvas->textureId, ImVec2(canvas->sx, canvas->sy));
			}
			ImGui::End();
			ImGui::PopID();
		}
	}
	guiContext.processEnd();
}

//

int main(int argc, char * argv[])
{
	setupPaths(CHIBI_RESOURCE_PATHS);
	
	s_app = new SimpleFrameworkApp("Example Browser", 800, 600);
	
	s_window = s_app->m_window;
	
	s_guiHelper = new FrameworkGUIHelperInterface();
	s_guiHelper->m_appInterface = s_app;
	s_guiHelper->m_renderInterface = s_app->m_renderer;
	s_guiHelper->m_parameterInterface = s_app->m_parameterInterface;
	s_guiHelper->m_2dCanvasInterface = s_app->m_2dCanvasInterface;
	
	ExampleEntriesAll examples;
	//MyExampleEntries examples;
	examples.initExampleEntries();
	
	auto * exampleBrowser = new FrameworkExampleBrowser(&examples);
	
	const bool init = exampleBrowser->init(argc, argv);
	
	exampleBrowser->registerFileImporter(".urdf", ImportURDFCreateFunc);
	exampleBrowser->registerFileImporter(".sdf", ImportSDFCreateFunc);
	exampleBrowser->registerFileImporter(".obj", ImportObjCreateFunc);
	exampleBrowser->registerFileImporter(".stl", ImportSTLCreateFunc);
	exampleBrowser->registerFileImporter(".bullet", SerializeBulletCreateFunc);

	if (init)
	{
		FrameworkImGuiContext guiContext;
		guiContext.init();
		
		for (;;)
		{
			auto * app = s_app;
			
			if (app->m_window->requestedExit())
				break;
			
			inputIsCaptured = false;
			
			doImGui(guiContext);
			
			for (auto & droppedFile : framework.droppedFiles)
			{
				openFileDemo(droppedFile.c_str());
			}
			
			app->m_renderer->init();
			app->m_renderer->updateCamera(app->getUpAxis());

			exampleBrowser->update(framework.timeStep);
			
			//
			
			pushBlend(BLEND_ALPHA);
			pushDepthTest(false, DEPTH_LESS);
			{
				projectScreen2d();
				guiContext.draw();
			}
			popDepthTest();
			popBlend();
			
			app->swapBuffer();
		}
		
		guiContext.shut();
	}

	delete exampleBrowser;
	exampleBrowser = nullptr;
	
	delete s_guiHelper;
	s_guiHelper = nullptr;
	
	delete s_app;
	s_app = nullptr;
}
