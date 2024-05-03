#include "MAlerts.hpp"
#include "MDocApplication.hpp"
#include "MColorPicker.hpp"
#include "MCommand.hpp"
#include "MControls.hpp"
#include "MMenu.hpp"
#include "MDocWindow.hpp"

#include "revision.hpp"

#include <coroutine>
#include <iostream>

const char kAppName[] = "ExampleApp";

class ExampleWindow : public MDocWindow
{
  public:
	ExampleWindow()
		: MDocWindow("Example", MRect{ 0, 0, 400, 400 }, kMPostionDefault | kMShowMenubar)
		, cClose(this, "close", &ExampleWindow::Close, 'W', kControlKey | kShiftKey)
		, cCut(this, "cut", &ExampleWindow::Cut, 'X', kControlKey | kShiftKey)
		, cCopy(this, "copy", &ExampleWindow::Copy, 'C', kControlKey | kShiftKey)
		, cPaste(this, "paste", &ExampleWindow::Paste, 'V', kControlKey | kShiftKey)
		, cSelectAll(this, "select-all", &ExampleWindow::SelectAll, 'A', kControlKey | kShiftKey)
		, eClicked(this, &ExampleWindow::Clicked)
		, eChanged(this, &ExampleWindow::Changed)
		, eColour(this, &ExampleWindow::Colour)
	{
		SetTitle("window-" + std::to_string(++s_nr));

		std::cout << GetBounds() << "\n";

		MBoxControl *vbox = new MBoxControl("vbox", MRect{}, false);
		vbox->SetLayout({ true, 0 });
		AddChild(vbox);

		MButton *btn = new MButton("test", MRect{ 10, 10, 100, 20 }, "Click me!");
		btn->SetLayout({ false, 4 });
		AddRoute(btn->eClicked, eClicked);
		vbox->AddChild(btn);

		MColorSwatch *cbtn = new MColorSwatch("kleur", MRect{ 0, 0, 20, 20 }, MColor("ffa348"));
		btn->SetLayout({ false, 4 });
		vbox->AddChild(cbtn);
		AddRoute(cbtn->eColorChanged, eColour);

		MCheckbox *cb = new MCheckbox("checkbox", MRect{}, "Een checkbox");
		btn->SetLayout({ false, 4 });
		AddRoute(cb->eValueChanged, eChanged);
		vbox->AddChild(cb);

		cCut.SetEnabled(mHasSelection);
		cCopy.SetEnabled(mHasSelection);
	}

	void Close()
	{
		MWindow::Close();
	}

	void Colour(const std::string &id, MColor inColor)
	{
		// SetColor
		// auto dlog = new MColorPicker(this, inColor);
		// dlog->Show();
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

// --------------------------------------------------------------------

class ExampleApp : public MDocApplication
{
  public:
	ExampleApp(MApplicationImpl *impl)
		: MDocApplication(impl)
		, cNew(this, "new", &ExampleApp::DoNew, 'N', kControlKey)
		, cAbout(this, "about", &ExampleApp::About)
		, cQuit(this, "quit", &ExampleApp::DoQuit, 'Q', kControlKey)
	{
	}

	void Initialise() override
	{
		MApplication::Initialise();
		MMenuBar::Init("example-menu");
	}

	// void Execute(const std::string &command, const std::vector<std::string> &arguments) override
	// {
	// 	if (command == "New")
	// 		DoNew();
	// }

	int HandleCommandLine(int argc, const char *const argv[]) override
	{
		DoNew();
		return 0;
	}

	void DoNew() 
	{
		auto w = new ExampleWindow();
		w->Select();
	}

	void DoQuit() override
	{
		DisplayAlert(nullptr, "close-all-windows-alert",
			[this](int inReply)
			{
				if (inReply == 1)
					MDocApplication::DoQuit();
			});
	}

	void About()
	{
		DisplayAlert(nullptr, "about-alert", { kVersionNumber, kRevisionGitTag, std::to_string(kBuildNumber), kRevisionDate });
	}

	MCommand<void()> cNew;
	MCommand<void()> cAbout;
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

	return ExampleApp::Main("com.hekkelman.libmgui-example", args);
}
