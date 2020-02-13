#pragma once

#include "CommonInterfaces/CommonWindowInterface.h"

class FrameworkWindow : public CommonWindowInterface
{
	float m_backgroundColor[3] = { 1.f, 1.f, 1.f };
	int m_windowSx = 0;
	int m_windowSy = 0;
	
	b3MouseMoveCallback m_mouseMoveCallback = nullptr;
	b3MouseButtonCallback m_mouseButtonCallback = nullptr;
	b3ResizeCallback m_resizeCallback = nullptr;
	b3WheelCallback m_wheelCallback = nullptr;
	b3KeyboardCallback m_keyboardCallback = nullptr;
	b3RenderCallback m_renderCallback = nullptr;
	
public:
	virtual ~FrameworkWindow() override final;

	virtual void createDefaultWindow(int width, int height, const char * title) override final;
	virtual void createWindow(const b3gWindowConstructionInfo & ci) override final;

	virtual void closeWindow() override final;

	virtual void runMainLoop() override final;
	virtual float getTimeInSeconds() override final;

	virtual bool requestedExit() const override final;
	virtual void setRequestExit() override final;

	virtual void startRendering() override final;

	virtual void endRendering() override final;

	virtual bool isModifierKeyPressed(int key) override final;

	virtual void setMouseMoveCallback(b3MouseMoveCallback mouseCallback) override final;
	virtual b3MouseMoveCallback getMouseMoveCallback() override final;

	virtual void setMouseButtonCallback(b3MouseButtonCallback mouseCallback) override final;
	virtual b3MouseButtonCallback getMouseButtonCallback() override final;

	virtual void setResizeCallback(b3ResizeCallback resizeCallback) override final;
	virtual b3ResizeCallback getResizeCallback() override final;

	virtual void setWheelCallback(b3WheelCallback wheelCallback) override final;
	virtual b3WheelCallback getWheelCallback() override final;

	virtual void setKeyboardCallback(b3KeyboardCallback keyboardCallback) override final;
	virtual b3KeyboardCallback getKeyboardCallback() override final;

	virtual void setRenderCallback(b3RenderCallback renderCallback) override final;

	virtual void setWindowTitle(const char* title) override final;

	virtual float getRetinaScale() const override final;
	virtual void setAllowRetina(bool allow) override final;

	virtual int getWidth() const override final;
	virtual int getHeight() const override final;

	virtual int fileOpenDialog(char * fileName, int maxFileNameLength) override final;
	
	void setBackgroundColor(float rgb[3]);
};
