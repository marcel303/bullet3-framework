#include "framework.h"
#include "FrameworkWindow.h"

FrameworkWindow::~FrameworkWindow()
{
	framework.shutdown();
}

void FrameworkWindow::createDefaultWindow(int width, int height, const char * title)
{
	framework.windowTitle = title;
	framework.init(width, height);
}

void FrameworkWindow::createWindow(const b3gWindowConstructionInfo & ci)
{
	createDefaultWindow(ci.m_width, ci.m_height, ci.m_title);
}

void FrameworkWindow::closeWindow()
{
	// ??
	Assert(false);
}

void FrameworkWindow::runMainLoop()
{
	framework.process();
	
	if (mouse.dx || mouse.dy)
	{
		if (m_mouseMoveCallback != nullptr)
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
}

void FrameworkWindow::endRendering()
{
	framework.endDraw();
}

bool FrameworkWindow::isModifierKeyPressed(int key)
{
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
		m_resizeCallback(getWidth(), getHeight());
}

b3ResizeCallback FrameworkWindow::getResizeCallback()
{
	return m_resizeCallback;
}

void FrameworkWindow::setWheelCallback(b3WheelCallback wheelCallback)
{
}

b3WheelCallback FrameworkWindow::getWheelCallback()
{
	return nullptr;
}

void FrameworkWindow::setKeyboardCallback(b3KeyboardCallback keyboardCallback)
{
}

b3KeyboardCallback FrameworkWindow::getKeyboardCallback()
{
	return nullptr;
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
	return 0;
}

void FrameworkWindow::setBackgroundColor(float rgb[3])
{
	m_backgroundColor[0] = rgb[0];
	m_backgroundColor[1] = rgb[1];
	m_backgroundColor[2] = rgb[2];
}
