#pragma once

#include "CommonInterfaces/CommonParameterInterface.h"
#include <vector>

class FrameworkParameterInterface : public CommonParameterInterface
{
public:
	enum ParamType
	{
		kParamType_Undefined,
		kParamType_Slider,
		kParamType_Button,
		kParamType_ComboBox
	};
	
	struct Param
	{
		ParamType type = kParamType_Undefined;
		
		SliderParams slider;
		ButtonParams button;
		ComboBoxParams comboBox;
		
		float value = 0.f;
		
		Param()
			: slider(nullptr, nullptr)
			, button(nullptr, 0, false)
			, comboBox()
		{
		}
	};
	
	std::vector<Param> m_params;
	
public:
	virtual ~FrameworkParameterInterface() override final { }

	virtual void registerSliderFloatParameter(SliderParams& params) override final
	{
		Param p;
		p.type = kParamType_Slider;
		p.slider = params;
		p.value = *params.m_paramValuePointer;
		m_params.push_back(p);
	}
	
	virtual void registerButtonParameter(ButtonParams& params) override final
	{
		Param p;
		p.type = kParamType_Button;
		p.button = params;
		p.value = params.m_initialState ? 1.f : 0.f;
		m_params.push_back(p);
	}
	
	virtual void registerComboBox(ComboBoxParams& params) override final
	{
		Param p;
		p.type = kParamType_ComboBox;
		p.comboBox = params;
		p.value = params.m_startItem;
		m_params.push_back(p);
	}

	virtual void syncParameters() override final
	{
		for (auto & param : m_params)
		{
			if (param.type == kParamType_Slider)
			{
				*param.slider.m_paramValuePointer = param.value;
				if (param.slider.m_callback != nullptr)
					param.slider.m_callback(param.value, param.slider.m_userPointer);
			}
			else if (param.type == kParamType_ComboBox)
			{
				if (param.comboBox.m_callback != nullptr)
				{
					param.comboBox.m_callback(
						param.comboBox.m_comboboxId,
						param.comboBox.m_items[(int)param.value],
						param.comboBox.m_userPointer);
				}
			}
		}
	}
	
	virtual void removeAllParameters() override final
	{
		m_params.clear();
	}
	
	virtual void setSliderValue(int sliderIndex, double sliderValue) override final { }
};
