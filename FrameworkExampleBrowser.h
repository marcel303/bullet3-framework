#pragma once

#include "../bullet3/examples/ExampleBrowser/ExampleBrowserInterface.h"

class ExampleEntries;
class SharedMemoryInterface;

class FrameworkExampleBrowser : public ExampleBrowserInterface
{
public:
	FrameworkExampleBrowser(ExampleEntries * examples);
	virtual ~FrameworkExampleBrowser() override final;

	virtual CommonExampleInterface * getCurrentExample() override final;

	virtual bool init(int argc, char * argv[]) override final;

	virtual void update(float deltaTime) override final;

	virtual bool requestedExit() override final;

	static void registerFileImporter(const char * extension, CommonExampleInterface::CreateFunc * createFunc);
};
