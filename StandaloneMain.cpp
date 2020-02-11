#include "CommonInterfaces/CommonExampleInterface.h"
#include "Utils/b3Clock.h"

#include "FrameworkGuiHelper.h"
#include "FrameworkRenderInterface.h"
#include "SimpleFrameworkApp.h"

#include "framework.h"

static CommonExampleInterface * example = nullptr;

static b3MouseMoveCallback prevMouseMoveCallback = nullptr;
static b3MouseButtonCallback prevMouseButtonCallback = nullptr;

static void OnMouseMove(float x, float y)
{
	const bool handled = example->mouseMoveCallback(x, y);

	if (!handled)
	{
		if (prevMouseMoveCallback)
			prevMouseMoveCallback(x, y);
	}
}

static void OnMouseDown(int button, int state, float x, float y)
{
	const bool handled = example->mouseButtonCallback(button, state, x, y);

	if (!handled)
	{
		if (prevMouseButtonCallback)
			prevMouseButtonCallback(button, state, x, y);
	}
}

int main(int argc, char * argv[])
{
	setupPaths(CHIBI_RESOURCE_PATHS);
	
	SimpleFrameworkApp * app = new SimpleFrameworkApp("Bullet Standalone Example", 1024, 768, true);

	prevMouseButtonCallback = app->m_window->getMouseButtonCallback();
	prevMouseMoveCallback = app->m_window->getMouseMoveCallback();

	app->m_window->setMouseButtonCallback((b3MouseButtonCallback)OnMouseDown);
	app->m_window->setMouseMoveCallback((b3MouseMoveCallback)OnMouseMove);
	
	FrameworkGUIHelperInterface gui;
	gui.m_renderInterface = app->m_renderer;
	gui.m_appInterface = app;
	
	CommonExampleOptions options(&gui);
	example = StandaloneExampleCreateFunc(options);
	example->processCommandLineArgs(argc, argv);

	example->initPhysics();
	example->resetCamera();

	b3Clock clock;

	for (;;)
	{
		if (app->m_window->requestedExit())
            break;
				
		app->m_renderer->init();
		app->m_renderer->updateCamera(app->getUpAxis());

		float dt = clock.getTimeInSeconds();
		if (dt < .1f)
			dt = .1f;

		example->stepSimulation(dt);
		clock.reset();

		example->physicsDebugDraw(~0);
		//example->renderScene();

		DrawGridData dg;
		dg.upAxis = app->getUpAxis();
		app->drawGrid(dg);

		app->swapBuffer();
	}

	example->exitPhysics();

	delete example;
    example = nullptr;
	
    delete app;
    app = nullptr;

	return 0;
}
