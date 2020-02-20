#pragma once

class btBroadphaseInterface;
class btCollisionConfiguration;
class btCollisionDispatcher;
class btMultiBodyConstraintSolver;
class btMultiBodyDynamicsWorld;

struct btOverlapFilterCallback;
class btOverlappingPairCache;

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
	btMultiBodyDynamicsWorld * dynamicsWorld = nullptr;

	void init(Settings & settings);
	void shut();
};
