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
	
	int textureId = 0;
};

struct FrameworkGraphicsInstance
{
	int shapeId = 0;
	
	Vec3 position;
	Vec4 rotation;
	Vec3 scaling = Vec3(1, 1, 1);
	
	Color color = colorWhite;
	Color specularColor = colorWhite;
	
	void calculateTransform(Mat4x4 & transform) const
	{
		transform =
			Mat4x4(true)
			.Translate(position[0], position[1], position[2])
			.Rotate(Quat(rotation[0], rotation[1], rotation[2], rotation[3]))
			.Scale(scaling[0], scaling[1], scaling[2]);
	}
};

//

static const int kMaxInstances = 64;

FrameworkGraphicsInstance * FrameworkRenderInterface::resolveGraphicsInstance(int id) const
{
	if (id < 0)
	{
		return nullptr;
	}
	else
	{
		auto i = m_graphicsInstances.find(id);
		Assert(i != m_graphicsInstances.end());
		
		if (i != m_graphicsInstances.end())
			return i->second;
		else
			return nullptr;
	}
}

void FrameworkRenderInterface::calculateViewMatrix(Mat4x4 & viewMatrix) const
{
	camera.getCameraViewMatrix(viewMatrix.m_v);
	viewMatrix = Mat4x4(true).Scale(1, 1, -1).Mul(viewMatrix);
}

FrameworkRenderInterface::~FrameworkRenderInterface()
{
	removeAllInstances();
	
	transformsBuffer->free();
	colorsBuffer->free();
	
	delete transformsBuffer;
	delete colorsBuffer;
}

void FrameworkRenderInterface::init()
{
	light.position.Set(-50, 30, 40);
	
	transformsBuffer = new ShaderBuffer();
	transformsBuffer->alloc(sizeof(Mat4x4) * kMaxInstances);

	colorsBuffer = new ShaderBuffer();
	colorsBuffer->alloc(sizeof(Color) * (2 * kMaxInstances));
}

void FrameworkRenderInterface::updateCamera(int upAxis)
{
	camera.setCameraUpAxis(upAxis);
	
	Mat4x4 viewMatrix;
	calculateViewMatrix(viewMatrix);
	gxSetMatrixf(GX_MODELVIEW, viewMatrix.m_v);
	updateCullFlip();
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
			
			shape->vb.free();
			shape->ib.free();
			
			delete shape;
			shape = nullptr;
		}
		
		m_graphicsShapes.clear();
	}
	
	// remove textures - nowhere else they seem to be removed!
	
	{
		for (auto & i : m_textures)
		{
			auto *& texture = i.second;
			
			texture->free();
			
			delete texture;
			texture = nullptr;
		}
		
		m_textures.clear();
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
	Assert(false); // not supported
}

void FrameworkRenderInterface::setLightPosition(const float lightPos[3])
{
	light.position.Set(lightPos[0], lightPos[1], lightPos[2]);
}

void FrameworkRenderInterface::setLightPosition(const double lightPos[3])
{
	const float lightPosf[3] = { (float)lightPos[0], (float)lightPos[1], (float)lightPos[2] };
	setLightPosition(lightPosf);
}

void FrameworkRenderInterface::setShadowMapResolution(int shadowMapResolution)
{
}

void FrameworkRenderInterface::setShadowMapWorldSize(float worldSize)
{
}

void FrameworkRenderInterface::setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16])
{
}

void FrameworkRenderInterface::setProjectiveTexture(bool useProjectiveTexture)
{
}

void FrameworkRenderInterface::renderScene()
{
	renderSceneInternal();
}

void FrameworkRenderInterface::renderSceneInternal(int renderMode)
{
#if 0
	// non-instanced code path. legacy. here for reference and troubleshooting
	Mat4x4 viewMatrix;
	gxGetMatrixf(GX_MODELVIEW, viewMatrix.m_v);
	
	const Vec3 lightPosition_view = viewMatrix.Mul4(light.position);
	
	pushCullMode(CULL_BACK, CULL_CW);
	Shader shader("shaders/bullet3-shape");
	setShader(shader);
	shader.setImmediate("u_lightPosition_view",
		lightPosition_view[0],
		lightPosition_view[1],
		lightPosition_view[2]);
	const GxImmediateIndex u_hasTex = shader.getImmediateIndex("u_hasTex");
	const GxImmediateIndex u_color = shader.getImmediateIndex("u_color");
	const GxImmediateIndex u_specularColor = shader.getImmediateIndex("u_specularColor");
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
			{
				const bool hasTex = shape->textureId > 0;
				const GxTextureId textureId = hasTex ? m_textures[shape->textureId]->id : 0;
				
				shader.setTexture("u_tex", 0, hasTex ? textureId : 0, true, false);
				shader.setImmediate(u_hasTex, hasTex ? 1.f : 0.f);
				shader.setImmediate(u_color,
					instance->color.r,
					instance->color.g,
					instance->color.b,
					instance->color.a);
				shader.setImmediate(u_specularColor,
					instance->specularColor.r,
					instance->specularColor.g,
					instance->specularColor.b);
				shape->mesh.drawInstanced(1);
			}
			gxPopMatrix();
		}
	}
	clearShader();
	popCullMode();
#else
	Mat4x4 projectionMatrix;
	gxGetMatrixf(GX_PROJECTION, projectionMatrix.m_v);
	
	Mat4x4 viewMatrix;
	gxGetMatrixf(GX_MODELVIEW, viewMatrix.m_v);
	
	const Vec3 lightPosition_view = viewMatrix.Mul4(light.position);
	
	pushCullMode(CULL_BACK, CULL_CW);
	Shader shader("shaders/bullet3-shape-instanced");
	setShader(shader);
	{
		shader.setImmediateMatrix4x4("u_viewMatrix", viewMatrix.m_v);
		shader.setImmediateMatrix4x4("u_viewProjectionMatrix", (projectionMatrix * viewMatrix).m_v);
		shader.setImmediate("u_lightPosition_view",
			lightPosition_view[0],
			lightPosition_view[1],
			lightPosition_view[2]);
		
		const GxImmediateIndex u_hasTex = shader.getImmediateIndex("u_hasTex");

		Mat4x4 transforms[kMaxInstances];
		Color * colors = (Color*)alloca(kMaxInstances * (sizeof(Color) * 2));
		
		int numInstances = 0;
		
		int batch_shapeId = -1;
		
		auto flush = [&]()
		{
			auto shape_itr = m_graphicsShapes.find(batch_shapeId);
			
			Assert(shape_itr != m_graphicsShapes.end());
			if (shape_itr == m_graphicsShapes.end())
				return;
			
			auto * shape = shape_itr->second;
			
			const bool hasTex = shape->textureId > 0;
			const GxTextureId textureId = hasTex ? m_textures[shape->textureId]->id : 0;

			shader.setTexture("u_tex", 0, hasTex ? textureId : 0, true, false);
			shader.setImmediate(u_hasTex, hasTex ? 1.f : 0.f);
			
			transformsBuffer->setData(&transforms, sizeof(Mat4x4) * numInstances);
			shader.setBuffer("transforms", *transformsBuffer);
			
			colorsBuffer->setData(colors, sizeof(Color) * (2 * numInstances));
			shader.setBuffer("colors", *colorsBuffer);

			shape->mesh.drawInstanced(numInstances);
			
			numInstances = 0;
		};
		
		// gather a list of all instances
		
		std::vector<FrameworkGraphicsInstance*> instances;
		instances.reserve(m_graphicsInstances.size());
		
		for (auto & instance_itr : m_graphicsInstances)
			instances.push_back(instance_itr.second);
		
		// sort the instances by shape
		
		std::sort(instances.begin(), instances.end(),
			[](const FrameworkGraphicsInstance * instance1, const FrameworkGraphicsInstance * instance2) -> bool
			{
				return instance1->shapeId < instance2->shapeId;
			});

		// draw instances, batched by shape

		for (auto * instance : instances)
		{
			if (instance->shapeId != batch_shapeId)
			{
				if (numInstances != 0)
					flush();
				
				batch_shapeId = instance->shapeId;
			}
			
			instance->calculateTransform(transforms[numInstances]);
			
			colors[numInstances * 2 + 0] = instance->color;
			colors[numInstances * 2 + 1] = instance->specularColor;
			
			numInstances++;
			
			if (numInstances == kMaxInstances)
				flush();
		}
		
		if (numInstances != 0)
			flush();
	}
	clearShader();
	popCullMode();
#endif
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
	Assert(shapeId > 0);
	
	const int id = m_nextGraphicsInstanceId++;
	Assert(m_graphicsInstances[id] == nullptr);
	
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
	const float positionf[3] = { (float)position[0], (float)position[1], (float)position[2] };
	const float quaternionf[4] = { (float)quaternion[0], (float)quaternion[1], (float)quaternion[2], (float)quaternion[3] };
	const float colorf[4] = { (float)color[0], (float)color[1], (float)color[2], (float)color[3] };
	const float scalingf[3] = { (float)scaling[0], (float)scaling[1], (float)scaling[2] };
	return registerGraphicsInstance(shapeId, positionf, quaternionf, colorf, scalingf);
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
	// todo : point sizes
	gxBegin(GX_POINTS);
	gxColor4f(color[0], color[1], color[2], 1.f); // fixme : seems like btVector3 is used as color, but btVector4 ..
	gxVertex3f(position[0], position[1], position[2]);
	gxEnd();
}

void FrameworkRenderInterface::drawPoint(const double* position, const double color[4], double pointDrawSize)
{
	const float positionf[3] = { (float)position[0], (float)position[1], (float)position[2] };
	const float colorf[4] = { (float)color[0], (float)color[1], (float)color[2], (float)color[3] };
	const float pointDrawSizef = pointDrawSize;
	drawPoint(positionf, colorf, pointDrawSizef);
}

void FrameworkRenderInterface::drawTexturedTriangleMesh(float worldPosition[3], float worldOrientation[4], const float* vertices, int numvertices, const unsigned int* indices, int numIndices, float color[4], int textureId, int vertexLayout)
{
	Assert(false); // todo : implement drawTexturedTriangleMesh
}

int FrameworkRenderInterface::registerShape(const float* vertices, int numvertices, const int* indices, int numIndices, int primitiveType, int textureId)
{
	const int id = m_nextGraphicsShapeId++;
	Assert(m_graphicsShapes[id] == nullptr);
	
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
	if (shapeId <= 0)
		return;
	
	auto i = m_graphicsShapes.find(shapeId);
	Assert(i != m_graphicsShapes.end());
	if (i != m_graphicsShapes.end())
	{
		auto * shape = i->second;
		
		const int numBytes = shape->vb.getNumBytes();
		
		shape->vb.free();
		shape->vb.alloc(vertices, numBytes);
	}
}

static uint8_t * convertTextureToRGBA(const uint8_t * src, int sx, int sy)
{
	uint8_t * dst = new uint8_t[sx * sy * 4];
	
	uint8_t * dst_ptr = dst;
	
	for (int i = sx * sy; i > 0; --i)
	{
		dst_ptr[0] = src[0];
		dst_ptr[1] = src[1];
		dst_ptr[2] = src[2];
		dst_ptr[3] = 0xff;
		
		dst_ptr += 4;
		src += 3;
	}
	
	return dst;
}

int FrameworkRenderInterface::registerTexture(const unsigned char* texels, int width, int height, bool flipPixelsY)
{
	const int id = m_nextTextureId++;
	Assert(m_textures[id] == nullptr);
	
	auto *& texture = m_textures[id];
	texture = new GxTexture();
	
	GxTextureProperties properties;
	properties.dimensions.sx = width;
	properties.dimensions.sy = height;
	properties.format = GX_RGBA8_UNORM;
	properties.mipmapped = true;
	texture->allocate(properties);
	
	auto * rgba = convertTextureToRGBA(texels, width, height);
	texture->upload(rgba, 4, 0);
	texture->generateMipmaps();
	delete [] rgba;
	
	return id;
}

void FrameworkRenderInterface::updateTexture(int textureId, const unsigned char* texels, bool flipPixelsY)
{
	Assert(textureId > 0);
	if (textureId <= 0)
		return;
	
	auto i = m_textures.find(textureId);
	Assert(i != m_textures.end());
	if (i != m_textures.end())
	{
		auto * texture = i->second;
		
		auto * rgba = convertTextureToRGBA(texels, texture->sx, texture->sy);
		texture->upload(rgba, 4, 0);
		texture->generateMipmaps();
		delete [] rgba;
	}
}

void FrameworkRenderInterface::activateTexture(int textureId)
{
	Assert(textureId > 0);
	if (textureId <= 0)
		return;
	
	auto i = m_textures.find(textureId);
	Assert(i != m_textures.end());
	if (i != m_textures.end())
	{
		auto * texture = i->second;
		
		gxSetTexture(texture->id, GX_SAMPLE_LINEAR, true);
	}
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

void FrameworkRenderInterface::removeTexture(int textureId)
{
	Assert(textureId > 0);
	if (textureId <= 0)
		return;
	
	auto i = m_textures.find(textureId);
	Assert(i != m_textures.end());
	if (i != m_textures.end())
	{
		auto *& texture = i->second;
		
		texture->free();
		
		delete texture;
		texture = nullptr;
		
		m_textures.erase(i);
	}
}

void FrameworkRenderInterface::setPlaneReflectionShapeIndex(int index)
{
}

int FrameworkRenderInterface::getShapeIndexFromInstance(int instanceId)
{
	auto * instance = resolveGraphicsInstance(instanceId);
	return instance ? instance->shapeId : -1;
}

bool FrameworkRenderInterface::readSingleInstanceTransformToCPU(float* position, float* orientation, int srcIndex)
{
	auto * instance = resolveGraphicsInstance(srcIndex);
	
	if (instance != nullptr)
	{
		memcpy(position, &instance->position, 3 * sizeof(float));
		memcpy(orientation, &instance->rotation, 4 * sizeof(float));
		return true;
	}
	else
	{
		return false;
	}
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
	const float positionf[3] = { (float)position[0], (float)position[1], (float)position[2] };
	const float orientationf[4] = { (float)orientation[0], (float)orientation[1], (float)orientation[2], (float)orientation[3] };
	writeSingleInstanceTransformToCPU(positionf, orientationf, srcIndex);
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
	const float colorf[4] = { (float)color[0], (float)color[1], (float)color[2], (float)color[3] };
	writeSingleInstanceColorToCPU(colorf, srcIndex);
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
	const float scalef[3] = { (float)scale[0], (float)scale[1], (float)scale[2] };
	writeSingleInstanceScaleToCPU(scalef, srcIndex);
}

void FrameworkRenderInterface::writeSingleInstanceSpecularColorToCPU(const double* specular, int srcIndex)
{
	const float specularf[3] = { (float)specular[0], (float)specular[1], (float)specular[2] };
	writeSingleInstanceSpecularColorToCPU(specularf, srcIndex);
}

void FrameworkRenderInterface::writeSingleInstanceSpecularColorToCPU(const float* specular, int srcIndex)
{
	auto * instance = resolveGraphicsInstance(srcIndex);
	
	if (instance != nullptr)
	{
		instance->specularColor.set(specular[0], specular[1], specular[2], 1.f);
	}
}

void FrameworkRenderInterface::writeSingleInstanceFlagsToCPU(int flags, int srcIndex)
{
	Assert(false); // todo : implement instance flags
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
	pushDepthTest(true, DEPTH_ALWAYS);
	pushColorWriteMask(0, 0, 0, 0);
	gxMatrixMode(GX_PROJECTION);
	gxPushMatrix();
	gxLoadIdentity();
	gxMatrixMode(GX_MODELVIEW);
	gxPushMatrix();
	gxLoadIdentity();
	{
		gxTranslatef(0, 0, 1.f);
		
		drawRect(-1, -1, +1, +1);
	}
	gxMatrixMode(GX_PROJECTION);
	gxPopMatrix();
	gxMatrixMode(GX_MODELVIEW);
	gxPopMatrix();
	popColorWriteMask();
	popDepthTest();
}

struct GLInstanceRendererInternalData * FrameworkRenderInterface::getInternalData()
{
	return nullptr;
}
