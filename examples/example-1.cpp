#include "MApplication.hpp"
#include "MCommand.hpp"
#include "MControls.hpp"
#include "MMenu.hpp"
#include "MWindow.hpp"

#include <iostream>

const char kAppName[] = "ExampleApp";

class ExampleWindow : public MWindow
{
  public:
	ExampleWindow()
		: MWindow("Example", MRect{0, 0, 400, 400}, kMPostionDefault | kMShowMenubar)
		, cClose(this, "close", &ExampleWindow::Close)
		, eClicked(this, &ExampleWindow::Clicked)
	{
		SetTitle("window-" + std::to_string(++s_nr));

		std::cout << GetBounds() << "\n";

		MButton *btn = new MButton("test", MRect{ 10, 10, 100, 20 }, "Click me!");
		btn->SetMargins(4, 4, 4, 4);
		AddRoute(btn->eClicked, eClicked);
		AddChild(btn);

	}

	void Close()
	{
		MWindow::Close();
	}

	void Clicked(const std::string &id)
	{
		std::cout << id << " clicked!\n";
	}

	MCommand<void()> cClose;
	MEventIn<void(const std::string &)> eClicked;
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
