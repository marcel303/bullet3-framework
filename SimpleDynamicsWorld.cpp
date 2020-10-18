#include "SimpleDynamicsWorld.h"

#include "BulletDynamics/Featherstone/btMultiBodyConstraintSolver.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftMultiBodyDynamicsWorld.h"

#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>

#include "FrameworkDebugDrawer.h"

void SimpleDynamicsWorld::init()
{
	Settings settings;
	
	init(settings);
}

void SimpleDynamicsWorld::init(Settings & settings)
{
	// create the collision detection broadphase and the constraint solver
	
	collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();

	collisionDispatcher = new btCollisionDispatcher(collisionConfiguration);

	pairCache = new btHashedOverlappingPairCache();
	pairCache->setOverlapFilterCallback(settings.filterCallback);

	broadphase = new btDbvtBroadphase(pairCache);

	constraintSolver = new btMultiBodyConstraintSolver();

	// create the dynamics world
	
	dynamicsWorld = new btSoftMultiBodyDynamicsWorld(
		collisionDispatcher,
		broadphase,
		constraintSolver,
		collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));

	// setup the soft body world info
	
	softBodyWorldInfo = new btSoftBodyWorldInfo();
	softBodyWorldInfo->m_broadphase = broadphase;
	softBodyWorldInfo->m_dispatcher = collisionDispatcher;
	softBodyWorldInfo->m_gravity = dynamicsWorld->getGravity();
	softBodyWorldInfo->m_sparsesdf.Initialize();
	softBodyWorldInfo->m_sparsesdf.setDefaultVoxelsz(0.25);
	
	// create the debug drawer

	debugDrawer = new FrameworkDebugDrawer();
	
	debugDrawer->setDebugMode(
		1*btIDebugDraw::DBG_DrawWireframe |
		0*btIDebugDraw::DBG_DrawAabb |
		0*btIDebugDraw::DBG_DrawContactPoints |
		1*btIDebugDraw::DBG_DrawNormals);
	
	/*
	debugDrawer->setDebugMode(
		1*btIDebugDraw::DBG_DrawWireframe |
		0*btIDebugDraw::DBG_DrawAabb |
		1*btIDebugDraw::DBG_DrawContactPoints |
		1*btIDebugDraw::DBG_DrawConstraints |
		1*btIDebugDraw::DBG_DrawConstraintLimits |
		1*btIDebugDraw::DBG_DrawNormals);
	*/
	dynamicsWorld->setDebugDrawer(debugDrawer);
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

	delete debugDrawer;
	debugDrawer = nullptr;
	
	delete softBodyWorldInfo;
	softBodyWorldInfo = nullptr;
	
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


btRigidBody * SimpleDynamicsWorld::createRigidBody(
	const btTransform & transform,
	float mass,
	btCollisionShape * shape,
	const btVector4 & color)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));
	
	const bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	auto * motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, motionState, shape, localInertia);
	auto * body = new btRigidBody(cInfo);

	body->setUserIndex(-1);
	
	dynamicsWorld->addRigidBody(body);
	
	softBodyWorldInfo->m_sparsesdf.Reset();
	
	return body;
}

static btSoftBody * createRope(
	btSoftBodyWorldInfo & worldInfo,
	const btVector3 & from,
	const btVector3 & to,
	const int numRopeSegments,
	int fixeds)
{
	const int numNodes = numRopeSegments + 1;
	
	btVector3 * x = new btVector3[numNodes];
	btScalar * m = new btScalar[numNodes];

	for (int i = 0; i < numNodes; ++i)
	{
		const btScalar t = i / btScalar(numNodes - 1);
		
		x[i] = lerp(from, to, t);
		m[i] = 1;
	}
	
	btSoftBody * psb = new btSoftBody(&worldInfo, numNodes, x, m);
	
	if (fixeds & 1) psb->setMass(0, 0);
	if (fixeds & 2) psb->setMass(numNodes - 1, 0);
	
	delete [] x;
	delete [] m;
	
	// create links
	
	for (int i = 1; i < numNodes; ++i)
	{
		psb->appendLink(i - 1, i);
	}
	
	return psb;
}

btSoftBody * SimpleDynamicsWorld::connectWithRope(btRigidBody * body1, btRigidBody * body2, const int numRopeSegments, const float stiffness)
{
	btSoftBody * softBodyRope0 = createRope(
		*softBodyWorldInfo,
		body1->getWorldTransform().getOrigin(),
		body2->getWorldTransform().getOrigin(),
		numRopeSegments,
		0);
	
	softBodyRope0->setTotalMass(0.1f);

	softBodyRope0->appendAnchor(0, body1);
	softBodyRope0->appendAnchor(softBodyRope0->m_nodes.size() - 1, body2);

	softBodyRope0->m_cfg.piterations = 5;
	softBodyRope0->m_cfg.kDP = 0.005f;
	softBodyRope0->m_cfg.kSHR = 1;
	softBodyRope0->m_cfg.kCHR = 1;
	softBodyRope0->m_cfg.kKHR = 1;
	
	for (int i = 0; i < softBodyRope0->m_materials.size(); ++i)
		softBodyRope0->m_materials[i]->m_kLST = stiffness; // LST = linear stiffness coefficient

	dynamicsWorld->addSoftBody(softBodyRope0);
	
	return softBodyRope0;
}

void SimpleDynamicsWorld::debugDraw()
{
	dynamicsWorld->debugDrawWorld();
}
