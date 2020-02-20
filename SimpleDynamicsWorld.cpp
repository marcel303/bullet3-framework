#include "SimpleDynamicsWorld.h"

#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.h"
#include "BulletDynamics/Featherstone/btMultiBodyConstraintSolver.h"

static btRigidBody * createRigidBody(
	btDynamicsWorld * dynamicsWorld,
	float mass,
	const btTransform & startTransform,
	btCollisionShape * shape,
	const btVector4 & color = btVector4(1, 0, 0, 1))
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));
	
	const bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	auto * myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);
	auto * body = new btRigidBody(cInfo);

	body->setUserIndex(-1);
	
	dynamicsWorld->addRigidBody(body);
	
	return body;
}

void SimpleDynamicsWorld::init(Settings & settings)
{
	// collision configuration contains default setup for memory, collision setup
	collisionConfiguration = new btDefaultCollisionConfiguration();

	collisionDispatcher = new btCollisionDispatcher(collisionConfiguration);

	pairCache = new btHashedOverlappingPairCache();
	pairCache->setOverlapFilterCallback(settings.filterCallback);

	broadphase = new btDbvtBroadphase(pairCache);

	constraintSolver = new btMultiBodyConstraintSolver();

	//
	
	dynamicsWorld = new btMultiBodyDynamicsWorld(
		collisionDispatcher,
		broadphase,
		constraintSolver,
		collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));

	//

#if 0
// todo : store debug drawer

	auto * debugDrawer = new FrameworkDebugDrawer();
	
	debugDrawer->setDebugMode(
		1*btIDebugDraw::DBG_DrawWireframe |
		0*btIDebugDraw::DBG_DrawAabb |
		0*btIDebugDraw::DBG_DrawContactPoints |
		1*btIDebugDraw::DBG_DrawNormals);
	
	dynamicsWorld->setDebugDrawer(debugDrawer);
#endif
}

void SimpleDynamicsWorld::shut()
{	
	if (dynamicsWorld != nullptr)
	{
		dynamicsWorld->setDebugDrawer(nullptr);

		for (int i = dynamicsWorld->getNumConstraints() - 1; i >= 0; i--)
		{
			dynamicsWorld->removeConstraint(dynamicsWorld->getConstraint(i));
		}

		for (int i = dynamicsWorld->getNumMultiBodyConstraints() - 1; i >= 0; i--)
		{
			btMultiBodyConstraint * mbc = dynamicsWorld->getMultiBodyConstraint(i);
			
			dynamicsWorld->removeMultiBodyConstraint(mbc);
			
			delete mbc;
		}

		for (int i = dynamicsWorld->getNumMultibodies() - 1; i >= 0; i--)
		{
			btMultiBody * mb = dynamicsWorld->getMultiBody(i);
			
			dynamicsWorld->removeMultiBody(mb);
			
			delete mb;
		}
		
		for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
		{
			btCollisionObject * obj = dynamicsWorld->getCollisionObjectArray()[i];
			btRigidBody * body = btRigidBody::upcast(obj);
			
			if (body != nullptr && body->getMotionState())
			{
				delete body->getMotionState();
			}
			
			dynamicsWorld->removeCollisionObject(obj);
			
			delete obj;
		}
	}

// todo
	//delete debugDrawer;
	//debugDrawer = nullptr;
	
	delete dynamicsWorld;
	dynamicsWorld = nullptr;

	delete constraintSolver;
	constraintSolver = nullptr;

	delete broadphase;
	broadphase = nullptr;

	delete collisionDispatcher;
	collisionDispatcher = nullptr;

	delete pairCache;
	pairCache = nullptr;

	delete collisionConfiguration;
	collisionConfiguration = nullptr;
}
