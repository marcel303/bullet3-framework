#include "Debugging.h"
#include "FrameworkCanvasInterface.h"
#include <map>

FrameworkCanvasInterface::~FrameworkCanvasInterface()
{
}

int FrameworkCanvasInterface::createCanvas(const char* canvasName, int width, int height, int xPos, int yPos)
{
	const int id = m_nextCanvasId++;
	Assert(m_canvases[id] == nullptr);
	
	auto *& canvas = m_canvases[id];
	canvas = new FrameworkCanvas();
	canvas->pixels = new uint8_t[width * height * 4];
	canvas->sx = width;
	canvas->sy = height;
	
	return id;
}

void FrameworkCanvasInterface::destroyCanvas(int canvasId)
{
	auto i = m_canvases.find(canvasId);
	Assert(i != m_canvases.end());
	if (i != m_canvases.end())
	{
		auto *& canvas = i->second;
		
		delete canvas;
		canvas = nullptr;
		
		m_canvases.erase(i);
	}
}

void FrameworkCanvasInterface::setPixel(int canvasId, int x, int y, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	auto i = m_canvases.find(canvasId);
	Assert(i != m_canvases.end());
	if (i != m_canvases.end())
	{
		auto * canvas = i->second;
	
		if (x >= 0 && x < canvas->sx &&
			y >= 0 && y < canvas->sy)
		{
			auto * line = canvas->getLine(y);
			
			line[x * 4 + 0] = red;
			line[x * 4 + 1] = green;
			line[x * 4 + 2] = blue;
			line[x * 4 + 3] = alpha;
		}
	}
}

void FrameworkCanvasInterface::getPixel(int canvasId, int x, int y, unsigned char& red, unsigned char& green, unsigned char& blue, unsigned char& alpha)
{
	auto i = m_canvases.find(canvasId);
	Assert(i != m_canvases.end());
	if (i != m_canvases.end())
	{
		auto * canvas = i->second;
		
		if (x >= 0 && x < canvas->sx &&
			y >= 0 && y < canvas->sy)
		{
			auto * line = canvas->getLine(y);
			
			red   = line[x * 4 + 0];
			green = line[x * 4 + 1];
			blue  = line[x * 4 + 2];
			alpha = line[x * 4 + 3];
			
			return;
		}
	}
	
	red = green = blue = alpha = 0xff;
}

void FrameworkCanvasInterface::refreshImageData(int canvasId)
{
	auto i = m_canvases.find(canvasId);
	Assert(i != m_canvases.end());
	if (i != m_canvases.end())
	{
		auto * canvas = i->second;
		
		freeTexture(canvas->textureId);
		
		canvas->textureId = createTextureFromRGBA8(canvas->pixels, canvas->sx, canvas->sy, false, true);
	}
}

//

#include "framework.h"

void FrameworkCanvasInterface::drawCanvas(const int x, const int y) const
{
	for (auto & i : m_canvases)
	{
		auto * canvas = i.second;
		
		if (canvas == nullptr)
			continue;
		
		gxPushMatrix();
		{
			gxTranslatef(x, y, 0);
			
			auto textureId = createTextureFromRGBA8(canvas->pixels, canvas->sx, canvas->sy, false, true);
			
			gxSetTexture(textureId);
			setColor(colorWhite);
			drawRect(0, 0, canvas->sx, canvas->sy);
			gxSetTexture(0);
			
			freeTexture(textureId);
		}
		gxPopMatrix();
	}
}
