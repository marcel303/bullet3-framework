#include "framework.h"
#include "FrameworkWindow.h"

FrameworkWindow::~FrameworkWindow()
{
}

void FrameworkWindow::createDefaultWindow(int width, int height, const char * title)
{
	framework.windowTitle = title;
	framework.windowIsResizable = true;
	framework.enableDepthBuffer = true;
	framework.msaaLevel = 4;
	framework.enableRealTimeEditing = true;
	framework.init(width, height);
	
	framework.getCurrentViewportSize(m_windowSx, m_windowSy);
}

void FrameworkWindow::createWindow(const b3gWindowConstructionInfo & ci)
{
	createDefaultWindow(ci.m_width, ci.m_height, ci.m_title);
}

void FrameworkWindow::closeWindow()
{
	framework.shutdown();
}

void FrameworkWindow::runMainLoop()
{
	framework.process();
	
	if (m_mouseMoveCallback != nullptr)
	{
		if (mouse.dx || mouse.dy)
		{
			m_mouseMoveCallback(mouse.x, mouse.y);
		}
	}
	
	if (m_mouseButtonCallback != nullptr)
	{
		if (mouse.wentDown(BUTTON_LEFT))
			m_mouseButtonCallback(0, 1, mouse.x, mouse.y);
		if (mouse.wentUp(BUTTON_LEFT))
			m_mouseButtonCallback(0, 0, mouse.x, mouse.y);
	}
	
	if (m_wheelCallback != nullptr)
	{
		if (mouse.scrollY != 0)
			m_wheelCallback(0, mouse.scrollY);
	}

	if (m_keyboardCallback != nullptr)
	{
	#if FRAMEWORK_USE_SDL
		for (auto & e : framework.events)
		{
			if (e.type == SDL_KEYDOWN)
				m_keyboardCallback(e.key.keysym.sym, 1);
			else if (e.type == SDL_KEYUP)
				m_keyboardCallback(e.key.keysym.sym, 0);
		}
	#endif
	}
	
	int viewSx;
	int viewSy;
	framework.getCurrentViewportSize(viewSx, viewSy);
	if (viewSx != m_windowSx || viewSy != m_windowSy)
	{
		m_windowSx = viewSx;
		m_windowSy = viewSy;
		
		if (m_resizeCallback != nullptr)
			m_resizeCallback(viewSx, viewSy);
	}
}

float FrameworkWindow::getTimeInSeconds()
{
	return framework.time;
}

bool FrameworkWindow::requestedExit() const
{
	return framework.quitRequested;
}

void FrameworkWindow::setRequestExit()
{
	framework.quitRequested = true;
}

void FrameworkWindow::startRendering()
{
	framework.beginDraw(
		m_backgroundColor[0] * 255.f,
		m_backgroundColor[1] * 255.f,
		m_backgroundColor[2] * 255.f,
		255);
	projectPerspective3d(90.f, .1f, 1000.f);
	pushDepthTest(true, DEPTH_LESS);
}

void FrameworkWindow::endRendering()
{
	popDepthTest();
	framework.endDraw();
}

bool FrameworkWindow::isModifierKeyPressed(int key)
{
	if (key == B3G_ALT)
		return keyboard.isDown(SDLK_LALT) || keyboard.isDown(SDLK_RALT);
	else if (key == B3G_CONTROL)
		return keyboard.isDown(SDLK_LCTRL) || keyboard.isDown(SDLK_RCTRL);
	else if (key == B3G_SHIFT)
		return keyboard.isDown(SDLK_LSHIFT) || keyboard.isDown(SDLK_RSHIFT);
	
	return false;
}

void FrameworkWindow::setMouseMoveCallback(b3MouseMoveCallback mouseCallback)
{
	m_mouseMoveCallback = mouseCallback;
}

b3MouseMoveCallback FrameworkWindow::getMouseMoveCallback()
{
	return m_mouseMoveCallback;
}

void FrameworkWindow::setMouseButtonCallback(b3MouseButtonCallback mouseCallback)
{
	m_mouseButtonCallback = mouseCallback;
}

b3MouseButtonCallback FrameworkWindow::getMouseButtonCallback()
{
	return m_mouseButtonCallback;
}

void FrameworkWindow::setResizeCallback(b3ResizeCallback resizeCallback)
{
	m_resizeCallback = resizeCallback;
	
	if (m_resizeCallback != nullptr)
	{
		m_resizeCallback(getWidth(), getHeight());
	}
}

b3ResizeCallback FrameworkWindow::getResizeCallback()
{
	return m_resizeCallback;
}

void FrameworkWindow::setWheelCallback(b3WheelCallback wheelCallback)
{
	m_wheelCallback = wheelCallback;
}

b3WheelCallback FrameworkWindow::getWheelCallback()
{
	return m_wheelCallback;
}

void FrameworkWindow::setKeyboardCallback(b3KeyboardCallback keyboardCallback)
{
	m_keyboardCallback = keyboardCallback;
}

b3KeyboardCallback FrameworkWindow::getKeyboardCallback()
{
	return m_keyboardCallback;
}

void FrameworkWindow::setRenderCallback(b3RenderCallback renderCallback)
{
	m_renderCallback = renderCallback;
}

void FrameworkWindow::setWindowTitle(const char* title)
{
	framework.getCurrentWindow().setTitle(title);
}

float FrameworkWindow::getRetinaScale() const
{
	return framework.getCurrentBackingScale();
}

void FrameworkWindow::setAllowRetina(bool allow)
{
	framework.allowHighDpi = allow;
}

int FrameworkWindow::getWidth() const
{
	int sx, sy;
	framework.getCurrentViewportSize(sx, sy);
	return sx;
}

int FrameworkWindow::getHeight() const
{
	int sx, sy;
	framework.getCurrentViewportSize(sx, sy);
	return sy;
}

int FrameworkWindow::fileOpenDialog(char * fileName, int maxFileNameLength)
{
	Assert(false);
	return 0;
}

void FrameworkWindow::setBackgroundColor(float rgb[3])
{
	m_backgroundColor[0] = rgb[0];
	m_backgroundColor[1] = rgb[1];
	m_backgroundColor[2] = rgb[2];
}
