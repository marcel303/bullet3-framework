#include "FrameworkDebugDrawer.h"

#include "framework.h"

FrameworkDebugDrawer::FrameworkDebugDrawer()
{
}

FrameworkDebugDrawer::~FrameworkDebugDrawer()
{
}

FrameworkDebugDrawer::DefaultColors FrameworkDebugDrawer::getDefaultColors() const
{
	return m_ourColors;
}

void FrameworkDebugDrawer::setDefaultColors(const DefaultColors & colors)
{
	m_ourColors = colors;
}

void FrameworkDebugDrawer::drawLine(
	const btVector3 & from,
	const btVector3 & to,
	const btVector3 & color)
{
	if (m_linePoints.size() >= BT_LINE_BATCH_SIZE)
	{
		flushLines();
	}

	const size_t index = m_linePoints.size();
	
	m_linePoints.push_back(from);
	m_linePoints.push_back(to);
	m_lineColors.push_back(color);
	m_lineColors.push_back(color);

	m_lineIndices.push_back(index);
	m_lineIndices.push_back(index + 1);
}

void FrameworkDebugDrawer::drawContactPoint(
	const btVector3 & PointOnB,
	const btVector3 & normalOnB,
	btScalar distance,
	int lifeTime,
	const btVector3 & color)
{
	drawLine(PointOnB, PointOnB + normalOnB * distance, color);
	btVector3 ncolor(0, 0, 0);
	drawLine(PointOnB, PointOnB + normalOnB * 0.01, ncolor);
}

void FrameworkDebugDrawer::reportErrorWarning(const char * warningString)
{
}

void FrameworkDebugDrawer::draw3dText(const btVector3 & location, const char * textString)
{
	Assert(false);
}

void FrameworkDebugDrawer::setDebugMode(int debugMode)
{
	m_debugMode = debugMode;
}

int FrameworkDebugDrawer::getDebugMode() const
{
	return m_debugMode;
}

void FrameworkDebugDrawer::flushLines()
{
	if (m_linePoints.size() > 0)
	{
		gxBegin(GX_LINES);
		{
			for (size_t i = 0; i < m_lineIndices.size(); ++i)
			{
				const auto index = m_lineIndices[i];
				
				gxColor4f(
					m_lineColors[index][0],
					m_lineColors[index][1],
					m_lineColors[index][2], 1.f);
				gxVertex3fv(&m_linePoints[index][0]);
			}
		}
		gxEnd();
		
		m_linePoints.clear();
		m_lineColors.clear();
		m_lineIndices.clear();
	}
}
