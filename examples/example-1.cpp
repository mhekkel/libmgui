#include "MAlerts.hpp"
#include "MApplication.hpp"
#include "MColorPicker.hpp"
#include "MCommand.hpp"
#include "MControls.hpp"
#include "MMenu.hpp"
#include "MWindow.hpp"

#include "revision.hpp"

#include <iostream>

const char kAppName[] = "ExampleApp";

class ExampleWindow : public MWindow
{
  public:
	ExampleWindow()
		: MWindow("Example", MRect{0, 0, 400, 400}, kMPostionDefault | kMShowMenubar)
		, cClose(this, "close", &ExampleWindow::Close, 'w', kControlKey | kShiftKey)
		, cCut(this, "cut", &ExampleWindow::Cut, 'x', kControlKey | kShiftKey)
		, cCopy(this, "copy", &ExampleWindow::Copy, 'c', kControlKey | kShiftKey)
		, cPaste(this, "paste", &ExampleWindow::Paste, 'v', kControlKey | kShiftKey)
		, cSelectAll(this, "select-all", &ExampleWindow::SelectAll, 'a', kControlKey | kShiftKey)
		, eClicked(this, &ExampleWindow::Clicked)
		, eChanged(this, &ExampleWindow::Changed)
		, eColour(this, &ExampleWindow::Colour)
	{
		SetTitle("window-" + std::to_string(++s_nr));

		std::cout << GetBounds() << "\n";

		MButton *btn = new MButton("test", MRect{ 10, 10, 100, 20 }, "Click me!");
		btn->SetMargins(4, 4, 4, 4);
		AddRoute(btn->eClicked, eClicked);
		AddChild(btn);

		MColorSwatch *cbtn = new MColorSwatch("kleur", MRect{0, 0, 100, 20}, MColor("ffa348"));
		cbtn->SetLayout(false, MRect{4, 4, 4, 4});
		AddChild(cbtn);
		AddRoute(cbtn->eColorChanged, eColour);

		MCheckbox *cb = new MCheckbox("checkbox", MRect{}, "Een checkbox");
		cb->SetMargins(4, 4, 4, 4);
		AddRoute(cb->eValueChanged, eChanged);
		AddChild(cb);



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

		std::cout << id << " clicked\n";
	}

	void Changed(const std::string &id, bool inState)
	{
		std::cout << id << " changed, status is nu " << std::boolalpha << inState << "\n";
	}

	void Cut() {}
	void Copy() {}
	void Paste() {}
	void SelectAll() {}

	MCommand<void()> cClose;

	MCommand<void()> cCut, cCopy, cPaste, cSelectAll;

	MEventIn<void(const std::string &)> eClicked;
	MEventIn<void(const std::string &, MColor)> eColour;

	MEventIn<void(const std::string &, bool)> eChanged;

	static int s_nr;

	bool mHasSelection = false;
};

int ExampleWindow::s_nr;

class ExampleApp : public MApplication
{
  public:
	ExampleApp(MApplicationImpl *impl)
		: MApplication(impl)
		, cNew(this, "new", &ExampleApp::DoNew, 'n', kControlKey | kShiftKey)
		, cQuit(this, "quit", &ExampleApp::Quit, 'q', kControlKey | kShiftKey)
		, cAbout(this, "about", &ExampleApp::About)
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
		DisplayAlert(nullptr, "close-all-windows-alert");

		// MApplication::DoQuit();
	}

	void About()
	{
		DisplayAlert(nullptr, "about-alert", { kVersionNumber, kRevisionGitTag, std::to_string(kBuildNumber), kRevisionDate });
	}

	MCommand<void()> cNew;
	MCommand<void()> cQuit;
	MCommand<void()> cAbout;
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
