#include "FrameworkDebugDrawer.h"

#include "framework.h"

FrameworkDebugDrawer::FrameworkDebugDrawer()
	: m_currentLineColor(-1, -1, -1)
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
	const btVector3 & from1,
	const btVector3 & to1,
	const btVector3 & color1)
{
	if (m_currentLineColor != color1 || m_linePoints.size() >= BT_LINE_BATCH_SIZE)
	{
		flushLines();
		m_currentLineColor = color1;
	}
	
	const Vec3 from(from1.x(), from1.y(), from1.z());
	const Vec3 to(to1.x(), to1.y(), to1.z());;

	const size_t index = m_linePoints.size();
	
	m_linePoints.push_back(from);
	m_linePoints.push_back(to);

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
		
		m_linePoints.clear();
		m_lineIndices.clear();
	}
}
