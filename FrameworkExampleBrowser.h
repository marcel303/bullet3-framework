#pragma once

#include "../bullet3/examples/ExampleBrowser/ExampleBrowserInterface.h"

class ExampleEntries;
class SharedMemoryInterface;

class FrameworkExampleBrowser : public ExampleBrowserInterface
{
public:
	FrameworkExampleBrowser(ExampleEntries * examples);
	virtual ~FrameworkExampleBrowser();

	virtual CommonExampleInterface * getCurrentExample();

	virtual bool init(int argc, char * argv[]);

	virtual void update(float deltaTime);

	virtual void updateGraphics();

	virtual bool requestedExit();

	virtual void setSharedMemoryInterface(SharedMemoryInterface * sharedMem);

	static void registerFileImporter(const char * extension, CommonExampleInterface::CreateFunc * createFunc);
};
