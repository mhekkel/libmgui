#include "MApplication.hpp"
#include "MCommand.hpp"
#include "MMenu.hpp"
#include "MWindow.hpp"

#include <iostream>

const char kAppName[] = "ExampleApp";

class ExampleWindow : public MWindow
{
  public:
	ExampleWindow()
		: MWindow("Example", MRect{}, kMPostionDefault | kMShowMenubar)
		, cClose(this, "close", &ExampleWindow::Close)
	{
		SetTitle("window-" + std::to_string(++s_nr));
	}

	void Close()
	{
		MWindow::Close();
	}

	MCommand<void()> cClose;
	static int s_nr;
};

int ExampleWindow::s_nr;

class ExampleApp : public MApplication
{
  public:
	ExampleApp(MApplicationImpl *impl)
		: MApplication(impl)
		, cNew(this, "new", &ExampleApp::DoNew)
		, cQuit(this, "quit", &ExampleApp::Quit)
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

	void Quit()
	{
		MApplication::DoQuit();
	}

	MCommand<void()> cNew;
	MCommand<void()> cQuit;
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
