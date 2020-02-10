#pragma once

#include "LinearMath/btIDebugDraw.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "LinearMath/btVector3.h"
#include "Vec3.h"

#define BT_LINE_BATCH_SIZE 512

ATTRIBUTE_ALIGNED16(class) FrameworkDebugDrawer : public btIDebugDraw
{
	int m_debugMode = 0;

	btAlignedObjectArray<btVector3> m_linePoints;
	btAlignedObjectArray<btVector3> m_lineColors;
	btAlignedObjectArray<int> m_lineIndices;

	DefaultColors m_ourColors;

public:
	BT_DECLARE_ALIGNED_ALLOCATOR();

	FrameworkDebugDrawer();
	virtual ~FrameworkDebugDrawer() override final;
	
	virtual DefaultColors getDefaultColors() const override final;
	virtual void setDefaultColors(const DefaultColors& colors) override final;

	virtual void drawLine(
		const btVector3 & from1,
		const btVector3 & to1,
		const btVector3& color1) override final;

	virtual void drawContactPoint(
		const btVector3 & PointOnB,
		const btVector3& normalOnB,
		btScalar distance,
		int lifeTime,
		const btVector3& color) override final;

	virtual void reportErrorWarning(const char* warningString) override final;
	
	virtual void draw3dText(const btVector3& location, const char* textString) override final;
	
	virtual void setDebugMode(int debugMode) override final;
	virtual int getDebugMode() const override final;

	virtual void flushLines() override final;
};
