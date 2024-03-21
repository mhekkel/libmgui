#include "MApplication.hpp"
#include "MMenu.hpp"
#include "MWindow.hpp"

#include <iostream>

const char kAppName[] = "ExampleApp";

class ExampleWindow : public MWindow
{
  public:
	ExampleWindow()
		: MWindow("Example", MRect{}, kMPostionDefault | kMShowMenubar)
	{
	}
};

class ExampleApp : public MApplication
{
  public:
	ExampleApp(MApplicationImpl *impl)
		: MApplication(impl)
	{

	}

	void Initialise() override
	{
		MApplication::Initialise();
		MMenuBar::Init("example-menu");
	}

	void Execute(const std::string &command, const std::vector<std::string> &arguments) override
	{
		if (command == "New")
			DoNew();
	}

	void DoNew() override
	{
		auto w = new ExampleWindow();
		w->Select();
	}
};

MApplication *MApplication::Create(MApplicationImpl *inImpl)
{
	return new ExampleApp(inImpl);
}

// --------------------------------------------------------------------

int main(int argc, char *argv[])
{
	std::vector<std::string> args;
	for (int i = 0; i < argc; ++i)
		args.emplace_back(argv[i]);

	return ExampleApp::Main(args);
}
