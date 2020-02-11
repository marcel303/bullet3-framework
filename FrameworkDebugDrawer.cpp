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
	m_linePoints[m_numLines*2+0] = from;
	m_linePoints[m_numLines*2+1] = to;
	m_lineColors[m_numLines] = color;

	m_numLines++;
	
	if (m_numLines == BT_LINE_BATCH_SIZE)
	{
		flushLines();
	}
}

void FrameworkDebugDrawer::drawContactPoint(
	const btVector3 & PointOnB,
	const btVector3 & normalOnB,
	btScalar distance,
	int lifeTime,
	const btVector3 & color)
{
	drawLine(PointOnB, PointOnB + normalOnB * distance, color);
	static const btVector3 ncolor(0, 0, 0);
	drawLine(PointOnB, PointOnB + normalOnB * 0.01f, ncolor);
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
	if (m_numLines > 0)
	{
		gxBegin(GX_LINES);
		{
			for (int i = 0; i < m_numLines; ++i)
			{
				gxColor4f(
					m_lineColors[i][0],
					m_lineColors[i][1],
					m_lineColors[i][2], 1.f);
				gxVertex3fv(&m_linePoints[i*2+0][0]);
				gxVertex3fv(&m_linePoints[i*2+1][0]);
			}
		}
		gxEnd();
		
		m_numLines = 0;
	}
}
