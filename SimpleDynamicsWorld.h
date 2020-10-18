#pragma once

#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.h"
#include "BulletSoftBody/btSoftMultiBodyDynamicsWorld.h"

class btBroadphaseInterface;
class btCollisionConfiguration;
class btCollisionDispatcher;
class btMultiBodyConstraintSolver;
class btSoftMultiBodyDynamicsWorld;
struct btSoftBodyWorldInfo;

struct btOverlapFilterCallback;
class btOverlappingPairCache;

class btIDebugDraw;

struct SimpleDynamicsWorld
{
	struct Settings
	{
		btOverlapFilterCallback * filterCallback = nullptr;
	};

	btCollisionConfiguration * collisionConfiguration = nullptr;
	btCollisionDispatcher * collisionDispatcher = nullptr;
	btOverlappingPairCache * pairCache = nullptr;
	btBroadphaseInterface * broadphase = nullptr;
	btMultiBodyConstraintSolver * constraintSolver = nullptr;
	btSoftMultiBodyDynamicsWorld * dynamicsWorld = nullptr;

	btSoftBodyWorldInfo * softBodyWorldInfo = nullptr;
	
	btIDebugDraw * debugDrawer = nullptr;
	
	void init();
	void init(Settings & settings);
	void shut();
	
	btRigidBody * createRigidBody(
		const btTransform & transform,
		float mass,
		btCollisionShape * shape,
		const btVector4 & color = btVector4(1, 0, 0, 1));
	
	btSoftBody * connectWithRope(
		btRigidBody * body1,
		btRigidBody * body2,
		const int numRopeSegments,
		const float stiffness);
	
	void debugDraw();
};
