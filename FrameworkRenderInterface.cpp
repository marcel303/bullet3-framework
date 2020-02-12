#include "FrameworkRenderInterface.h"

#include "Debugging.h"
#include "framework.h"

//

#include "data/engine/ShaderCommon.txt"
#include "gx_mesh.h"
#include "Quat.h"
#include <map>

struct FrameworkGraphicsShape
{
	GxVertexBuffer vb;
	GxIndexBuffer ib;
	GxMesh mesh;
	
	GxTextureId textureId = 0;
};

struct FrameworkGraphicsInstance
{
	int shapeId = 0;
	
	Vec3 position;
	Vec4 rotation;
	Vec3 scaling = Vec3(1, 1, 1);
	
	Color color = colorWhite;
	
	void calculateTransform(Mat4x4 & transform) const
	{
		transform =
			Mat4x4(true)
			.Translate(position[0], position[1], position[2])
			.Rotate(Quat(rotation[0], rotation[1], rotation[2], rotation[3]))
			.Scale(scaling[0], scaling[1], scaling[2]);
	}
};

// todo : move to internal data
std::map<int, FrameworkGraphicsShape*> m_graphicsShapes;
static int m_nextGraphicsShapeId = 1;

std::map<int, FrameworkGraphicsInstance*> m_graphicsInstances;
static int m_nextGraphicsInstanceId = 1;

static FrameworkGraphicsInstance * resolveGraphicsInstance(int id)
{
	auto i = m_graphicsInstances.find(id);
	Assert(i != m_graphicsInstances.end());
	
	if (i != m_graphicsInstances.end())
		return i->second;
	else
		return nullptr;
}

//

void FrameworkRenderInterface::calculateViewMatrix(Mat4x4 & viewMatrix) const
{
	camera.getCameraViewMatrix(viewMatrix.m_v);
	viewMatrix = Mat4x4(true).Scale(1, 1, -1).Mul(viewMatrix);
}

FrameworkRenderInterface::~FrameworkRenderInterface()
{
}

void FrameworkRenderInterface::init()
{
	light.position.Set(-50, 30, 40);
}

void FrameworkRenderInterface::updateCamera(int upAxis)
{
	camera.setCameraUpAxis(upAxis);
	
	Mat4x4 viewMatrix;
	calculateViewMatrix(viewMatrix);
	gxSetMatrixf(GX_MODELVIEW, viewMatrix.m_v);
}

void FrameworkRenderInterface::removeAllInstances()
{
	// remove instances
	
	while (!m_graphicsInstances.empty())
	{
		removeGraphicsInstance(m_graphicsInstances.begin()->first);
	}
	
	// remove shapes
	
	{
		for (auto & i : m_graphicsShapes)
		{
			auto *& shape = i.second;
			
			delete shape;
			shape = nullptr;
		}
		
		m_graphicsShapes.clear();
	}
}

void FrameworkRenderInterface::removeGraphicsInstance(int id)
{
	auto i = m_graphicsInstances.find(id);
	Assert(i != m_graphicsInstances.end());
	
	if (i != m_graphicsInstances.end())
	{
		delete i->second;
		i->second = nullptr;
		
		m_graphicsInstances.erase(i);
	}
}

const CommonCameraInterface * FrameworkRenderInterface::getActiveCamera() const
{
	return &camera;
}

CommonCameraInterface * FrameworkRenderInterface::getActiveCamera()
{
	return &camera;
}

void FrameworkRenderInterface::setActiveCamera(CommonCameraInterface * cam)
{
	Assert(false);
}

void FrameworkRenderInterface::setLightPosition(const float lightPos[3])
{
	light.position.Set(lightPos[0], lightPos[1], lightPos[2]);
}

void FrameworkRenderInterface::setLightPosition(const double lightPos[3])
{
	Assert(false);
}

void FrameworkRenderInterface::setShadowMapResolution(int shadowMapResolution)
{
	Assert(false);
}

void FrameworkRenderInterface::setShadowMapWorldSize(float worldSize)
{
	Assert(false);
}

void FrameworkRenderInterface::setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16])
{
	Assert(false);
}

void FrameworkRenderInterface::setProjectiveTexture(bool useProjectiveTexture)
{
	Assert(false);
}

void FrameworkRenderInterface::renderScene()
{
	renderSceneInternal();
}

void FrameworkRenderInterface::renderSceneInternal(int renderMode)
{
	Mat4x4 viewMatrix;
	calculateViewMatrix(viewMatrix);
	
	const Vec3 lightPosition_view = viewMatrix.Mul4(light.position);
	
	for (auto & i : m_graphicsInstances)
	{
		auto * instance = i.second;
		
		auto shape_itr = m_graphicsShapes.find(instance->shapeId);
		Assert(shape_itr != m_graphicsShapes.end());
		if (shape_itr != m_graphicsShapes.end())
		{
			auto * shape = shape_itr->second;
			
			Mat4x4 transform;
			instance->calculateTransform(transform);
			
			gxPushMatrix();
			gxMultMatrixf(transform.m_v);
			Shader shader("shaders/bullet3-shape");
			setShader(shader);
			{
				const bool hasTex = shape->textureId != 0 && shape->textureId != (GxTextureId)-1;
				
				shader.setTexture("u_tex", 0, hasTex ? shape->textureId : 0, true, true);
				shader.setImmediate("u_hasTex", hasTex ? 1.f : 0.f);
				shader.setImmediate("u_color",
					instance->color.r,
					instance->color.g,
					instance->color.b,
					instance->color.a);
					shader.setImmediate("u_lightPosition_view",
						lightPosition_view[0],
						lightPosition_view[1],
						lightPosition_view[2]);
				shape->mesh.draw();
			}
			clearShader();
			gxPopMatrix();
		}
	}
}

int FrameworkRenderInterface::getScreenWidth()
{
	return m_screenWidth;
}

int FrameworkRenderInterface::getScreenHeight()
{
	return m_screenHeight;
}

void FrameworkRenderInterface::resize(int width, int height)
{
	m_screenWidth = width;
	m_screenHeight = height;
}

int FrameworkRenderInterface::registerGraphicsInstance(int shapeId, const float* position, const float* quaternion, const float* color, const float* scaling)
{
	const int id = m_nextGraphicsInstanceId++;
	
	auto *& instance = m_graphicsInstances[id];
	instance = new FrameworkGraphicsInstance();
	instance->shapeId = shapeId;
	instance->position.Set(position[0], position[1], position[2]);
	instance->rotation.Set(quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
	instance->scaling.Set(scaling[0], scaling[1], scaling[2]);
	instance->color = Color(color[0], color[1], color[2], color[3]);
	
	return id;
}

int FrameworkRenderInterface::registerGraphicsInstance(int shapeId, const double* position, const double* quaternion, const double* color, const double* scaling)
{
 	Assert(false);
 	return 0;
}

void FrameworkRenderInterface::drawLines(const float* positions, const float color[4], int numPoints, int pointStrideInBytes, const unsigned int* indices, int numIndices, float pointDrawSize)
{
	gxColor4fv(color);
	gxBegin(GX_LINES);
	{
		const uint8_t * positions_mem = (uint8_t*)positions;
		
		for (int i = 0; i < numIndices; ++i)
		{
			const int index = indices[i];
			
			const uint8_t * position_ptr = positions_mem + index * pointStrideInBytes;
			const float * position = (float*)position_ptr;
			
			gxVertex4fv(position);
		}
	}
	gxEnd();
}

void FrameworkRenderInterface::drawLine(const float from[4], const float to[4], const float color[4], float lineWidth)
{
	gxBegin(GX_LINES);
	{
		gxColor4fv(color);
		gxVertex4fv(from);
		gxVertex4fv(to);
	}
	gxEnd();
}

void FrameworkRenderInterface::drawLine(const double from[4], const double to[4], const double color[4], double lineWidth)
{
	gxBegin(GX_LINES);
	{
		gxColor4f(color[0], color[1], color[2], color[3]);
		gxVertex4f(from[0], from[1], from[2], from[3]);
		gxVertex4f(to[0], to[1], to[2], to[3]);
	}
	gxEnd();
}

void FrameworkRenderInterface::drawPoint(const float* position, const float color[4], float pointDrawSize)
{
	Assert(false);
}

void FrameworkRenderInterface::drawPoint(const double* position, const double color[4], double pointDrawSize)
{
	Assert(false);
}

void FrameworkRenderInterface::drawTexturedTriangleMesh(float worldPosition[3], float worldOrientation[4], const float* vertices, int numvertices, const unsigned int* indices, int numIndices, float color[4], int textureId, int vertexLayout)
{
	Assert(false);
}

int FrameworkRenderInterface::registerShape(const float* vertices, int numvertices, const int* indices, int numIndices, int primitiveType, int textureId)
{
	const int id = m_nextGraphicsShapeId++;
	
	auto *& shape = m_graphicsShapes[id];
	shape = new FrameworkGraphicsShape();
	shape->vb.alloc(vertices, numvertices * sizeof(float) * 9);
	shape->ib.alloc(indices, numIndices, GX_INDEX_32);
	
	const GxVertexInput vsInputs[3] =
		{
			{ VS_POSITION,  4, GX_ELEMENT_FLOAT32, false, sizeof(float)*0, sizeof(float)*9 },
			{ VS_NORMAL,    3, GX_ELEMENT_FLOAT32, false, sizeof(float)*4, sizeof(float)*9 },
			{ VS_TEXCOORD0, 2, GX_ELEMENT_FLOAT32, false, sizeof(float)*7, sizeof(float)*9 }
		};
	
	shape->mesh.setVertexBuffer(&shape->vb, vsInputs, 3, sizeof(float)*9);
	shape->mesh.setIndexBuffer(&shape->ib);
	
	const GX_PRIMITIVE_TYPE gxPrimitiveType =
	 	primitiveType == B3_GL_TRIANGLES ? GX_TRIANGLES :
	 	primitiveType == B3_GL_POINTS ? GX_POINTS :
	 	(GX_PRIMITIVE_TYPE)-1;
	
	if (numIndices > 0)
		shape->mesh.addPrim(gxPrimitiveType, numIndices, true);
	else
		shape->mesh.addPrim(gxPrimitiveType, numvertices, false);
	
	shape->textureId = textureId;
	
	return id;
}

void FrameworkRenderInterface::updateShape(int shapeId, const float* vertices)
{
	Assert(false);
	
	if (shapeId <= 0)
		return;
}

int FrameworkRenderInterface::registerTexture(const unsigned char* texels, int width, int height, bool flipPixelsY)
{
	return createTextureFromRGB8(texels, width, height, true, true);
}

void FrameworkRenderInterface::updateTexture(int textureId, const unsigned char* texels, bool flipPixelsY)
{
	Assert(false);
}

void FrameworkRenderInterface::activateTexture(int textureId)
{
	gxSetTexture(textureId);
}

void FrameworkRenderInterface::replaceTexture(int shapeId, int textureId)
{
	auto i = m_graphicsShapes.find(shapeId);
	Assert(i != m_graphicsShapes.end());
	if (i != m_graphicsShapes.end())
	{
		auto * shape = i->second;
		
		shape->textureId = textureId;
	}
}

void FrameworkRenderInterface::removeTexture(int id)
{
	GxTextureId textureId = id;
	
	freeTexture(textureId);
}

void FrameworkRenderInterface::setPlaneReflectionShapeIndex(int index)
{
	Assert(false);
}

int FrameworkRenderInterface::getShapeIndexFromInstance(int instanceId)
{
	auto * instance = resolveGraphicsInstance(instanceId);
	return instance ? instance->shapeId : -1;
}

bool FrameworkRenderInterface::readSingleInstanceTransformToCPU(float* position, float* orientation, int srcIndex)
{
	Assert(false);
	return false;
}

void FrameworkRenderInterface::writeSingleInstanceTransformToCPU(const float* position, const float* orientation, int srcIndex)
{
	auto * instance = resolveGraphicsInstance(srcIndex);
	
	if (instance != nullptr)
	{
		instance->position.Set(position[0], position[1], position[2]);
		instance->rotation.Set(orientation[0], orientation[1], orientation[2], orientation[3]);
	}
}

void FrameworkRenderInterface::writeSingleInstanceTransformToCPU(const double* position, const double* orientation, int srcIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::writeSingleInstanceColorToCPU(const float* color, int srcIndex)
{
	auto * instance = resolveGraphicsInstance(srcIndex);
	
	if (instance != nullptr)
	{
		instance->color.set(color[0], color[1], color[2], color[3]);
	}
}

void FrameworkRenderInterface::writeSingleInstanceColorToCPU(const double* color, int srcIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::writeSingleInstanceScaleToCPU(const float* scale, int srcIndex)
{
	auto * instance = resolveGraphicsInstance(srcIndex);
	
	if (instance != nullptr)
	{
		instance->scaling.Set(scale[0], scale[1], scale[2]);
	}
}

void FrameworkRenderInterface::writeSingleInstanceScaleToCPU(const double* scale, int srcIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::writeSingleInstanceSpecularColorToCPU(const double* specular, int srcIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::writeSingleInstanceSpecularColorToCPU(const float* specular, int srcIndex)
{
	Assert(false);
}

void FrameworkRenderInterface::writeSingleInstanceFlagsToCPU(int flags, int srcIndex)
{
	Assert(false);
}

int FrameworkRenderInterface::getTotalNumInstances() const
{
	return m_graphicsInstances.size();
}

void FrameworkRenderInterface::writeTransforms()
{
}

void FrameworkRenderInterface::clearZBuffer()
{
	Assert(false);
}

struct GLInstanceRendererInternalData * FrameworkRenderInterface::getInternalData()
{
	return nullptr;
}
