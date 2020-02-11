#pragma once

#include "CommonInterfaces/CommonParameterInterface.h"

class FrameworkParameterInterface : public CommonParameterInterface
{
public:
	virtual ~FrameworkParameterInterface() override final { }

	virtual void registerSliderFloatParameter(SliderParams& params) override final { }
	virtual void registerButtonParameter(ButtonParams& params) override final { }
	virtual void registerComboBox(ComboBoxParams& params) override final { }

	virtual void syncParameters() override final { }
	virtual void removeAllParameters() override final { }
	virtual void setSliderValue(int sliderIndex, double sliderValue) override final { }
};
