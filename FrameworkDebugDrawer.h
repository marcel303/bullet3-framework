#pragma once

#include "framework.h"

#include "LinearMath/btIDebugDraw.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "LinearMath/btVector3.h"

#define BT_LINE_BATCH_SIZE 512

ATTRIBUTE_ALIGNED16(class)
FrameworkDebugDrawer : public btIDebugDraw
{
	//CommonGraphicsApp * m_app = nullptr;
	int m_debugMode = 0;

	btAlignedObjectArray<Vec3> m_linePoints;
	btAlignedObjectArray<unsigned int> m_lineIndices;

	btVector3 m_currentLineColor;
	DefaultColors m_ourColors;

public:
	BT_DECLARE_ALIGNED_ALLOCATOR();

	FrameworkDebugDrawer()//CommonGraphicsApp * app)
		//: m_app(app)
		: m_debugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawAabb)
		, m_currentLineColor(-1, -1, -1)
	{
	}

	virtual ~FrameworkDebugDrawer() override final
	{
	}
	
	virtual DefaultColors getDefaultColors() const override final
	{
		return m_ourColors;
	}
	
	///the default implementation for setDefaultColors has no effect. A derived class can implement it and store the colors.
	virtual void setDefaultColors(const DefaultColors& colors) override final
	{
		m_ourColors = colors;
	}

	virtual void drawLine(
		const btVector3 & from1,
		const btVector3 & to1,
		const btVector3& color1) override final
	{
		if (m_currentLineColor != color1 || m_linePoints.size() >= BT_LINE_BATCH_SIZE)
		{
			flushLines();
			m_currentLineColor = color1;
		}
		
		Vec3 from(from1.x(), from1.y(), from1.z());
		Vec3 to(to1.x(), to1.y(), to1.z());;

		const size_t index = m_linePoints.size();
		
		m_linePoints.push_back(from);
		m_linePoints.push_back(to);

		m_lineIndices.push_back(index);
		m_lineIndices.push_back(index + 1);
	}

	virtual void drawContactPoint(
		const btVector3 & PointOnB,
		const btVector3& normalOnB,
		btScalar distance,
		int lifeTime,
		const btVector3& color) override final
	{
		drawLine(PointOnB, PointOnB + normalOnB * distance, color);
		btVector3 ncolor(0, 0, 0);
		drawLine(PointOnB, PointOnB + normalOnB * 0.01, ncolor);
	}

	virtual void reportErrorWarning(const char* warningString) override final
	{
	}

	virtual void draw3dText(const btVector3& location, const char* textString) override final
	{
	}

	virtual void setDebugMode(int debugMode) override final
	{
		m_debugMode = debugMode;
	}

	virtual int getDebugMode() const override final
	{
		return m_debugMode;
	}

	virtual void flushLines() override final
	{
		int sz = m_linePoints.size();
		
		if (sz)
		{
			float debugColor[4];
			debugColor[0] = m_currentLineColor.x();
			debugColor[1] = m_currentLineColor.y();
			debugColor[2] = m_currentLineColor.z();
			debugColor[3] = 1.f;
			
			gxBegin(GX_LINES);
			{
				gxColor4fv(debugColor);
				
				for (size_t i = 0; i < m_lineIndices.size(); ++i)
				{
					const int index = m_lineIndices[i];
					
					gxVertex3fv(&m_linePoints[index][0]);
				}
			}
			gxEnd();
			/*
			m_app->m_renderer->drawLines(
				&m_linePoints[0].x, debugColor,
				m_linePoints.size(), sizeof(MyDebugVec3),
				&m_lineIndices[0],
				m_lineIndices.size(),
				1);
			*/
			
			m_linePoints.clear();
			m_lineIndices.clear();
		}
	}
};
