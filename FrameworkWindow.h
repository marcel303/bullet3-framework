#pragma once

#include "CommonInterfaces/CommonWindowInterface.h"

class Window;

class FrameworkWindow : public CommonWindowInterface
{
	float m_backgroundColor[3] = { 1.f, 1.f, 1.f };
	
	b3MouseMoveCallback m_mouseMoveCallback = nullptr;
	b3MouseButtonCallback m_mouseButtonCallback = nullptr;
	b3ResizeCallback m_resizeCallback = nullptr;
	b3RenderCallback m_renderCallback = nullptr;
	
public:
	virtual ~FrameworkWindow() override;

	virtual void createDefaultWindow(int width, int height, const char * title) override;
	virtual void createWindow(const b3gWindowConstructionInfo & ci) override;

	virtual void closeWindow() override;

	virtual void runMainLoop() override;
	virtual float getTimeInSeconds() override;

	virtual bool requestedExit() const override;
	virtual void setRequestExit() override;

	virtual void startRendering() override;

	virtual void endRendering() override;

	virtual bool isModifierKeyPressed(int key) override;

	virtual void setMouseMoveCallback(b3MouseMoveCallback mouseCallback) override;
	virtual b3MouseMoveCallback getMouseMoveCallback() override;

	virtual void setMouseButtonCallback(b3MouseButtonCallback mouseCallback) override;
	virtual b3MouseButtonCallback getMouseButtonCallback() override;

	virtual void setResizeCallback(b3ResizeCallback resizeCallback) override;
	virtual b3ResizeCallback getResizeCallback() override;

	virtual void setWheelCallback(b3WheelCallback wheelCallback) override;
	virtual b3WheelCallback getWheelCallback() override;

	virtual void setKeyboardCallback(b3KeyboardCallback keyboardCallback) override;
	virtual b3KeyboardCallback getKeyboardCallback() override;

	virtual void setRenderCallback(b3RenderCallback renderCallback) override;

	virtual void setWindowTitle(const char* title) override;

	virtual float getRetinaScale() const override;
	virtual void setAllowRetina(bool allow) override;

	virtual int getWidth() const override;
	virtual int getHeight() const override;

	virtual int fileOpenDialog(char * fileName, int maxFileNameLength) override;
	
	void setBackgroundColor(float rgb[3]);
};
