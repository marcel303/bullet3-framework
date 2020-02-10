#pragma once

#include "CommonInterfaces/CommonParameterInterface.h"

class FrameworkParameterInterface : public CommonParameterInterface
{
public:
	virtual ~FrameworkParameterInterface() override { }

	virtual void registerSliderFloatParameter(SliderParams& params) override { }
	virtual void registerButtonParameter(ButtonParams& params) override { }
	virtual void registerComboBox(ComboBoxParams& params) override { }

	virtual void syncParameters() override { }
	virtual void removeAllParameters() override { }
	virtual void setSliderValue(int sliderIndex, double sliderValue) override { }
};
