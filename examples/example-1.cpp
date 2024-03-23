#include "MApplication.hpp"
#include "MColorPicker.hpp"
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
		, cCut(this, "cut", &ExampleWindow::Cut)
		, cCopy(this, "copy", &ExampleWindow::Copy)
		, cPaste(this, "paste", &ExampleWindow::Paste)
		, cSelectAll(this, "select-all", &ExampleWindow::SelectAll)
		, eClicked(this, &ExampleWindow::Clicked)
		, eColour(this, &ExampleWindow::Colour)
	{
		SetTitle("window-" + std::to_string(++s_nr));

		std::cout << GetBounds() << "\n";

		MButton *btn = new MButton("test", MRect{ 10, 10, 100, 20 }, "Click me!");
		btn->SetMargins(4, 4, 4, 4);
		AddRoute(btn->eClicked, eClicked);
		AddChild(btn);

		MColorSwatch *cbtn = new MColorSwatch("kleur", MRect{0, 0, 100, 20}, kBlack);
		cbtn->SetLayout(false, MRect{4, 4, 4, 4});
		AddChild(cbtn);
		AddRoute(cbtn->eColorChanged, eColour);

		cCut.SetEnabled(mHasSelection);
		cCopy.SetEnabled(mHasSelection);
	}

	void Close()
	{
		MWindow::Close();
	}

	void Colour(const std::string &id, MColor inColor)
	{
		auto dlog = new MColorPicker(this, inColor);
		dlog->Show();
	}

	void Clicked(const std::string &id)
	{
		mHasSelection = not mHasSelection;

		cCut.SetEnabled(mHasSelection);
		cCopy.SetEnabled(mHasSelection);

		std::cout << id << " clicked!\n";
	}

	void Cut() {}
	void Copy() {}
	void Paste() {}
	void SelectAll() {}

	MCommand<void()> cClose;

	MCommand<void()> cCut, cCopy, cPaste, cSelectAll;

	MEventIn<void(const std::string &)> eClicked;
	MEventIn<void(const std::string &, MColor)> eColour;

	static int s_nr;

	bool mHasSelection = false;
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
