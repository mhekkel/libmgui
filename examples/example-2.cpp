#include "MAlerts.hpp"
#include "MColorPicker.hpp"
#include "MCommand.hpp"
#include "MControls.hpp"
#include "MDevice.hpp"
#include "MDocApplication.hpp"
#include "MDocWindow.hpp"
#include "MMenu.hpp"

#include "revision.hpp"

#include <coroutine>
#include <iostream>

const char kAppName[] = "ExampleApp";

const MColor
	kTextColor,
	kKeyWordColor("#3d4c9e"),
	kPreProcessorColor("#005454"),
	kCharConstColor("#ad6739"),
	kCommentColor("#9b2e35"),
	kStringColor("#666666"),
	kTagColor("#008484"),
	kAttribColor("#1e843b"),
	kInvisiblesColor("#aaaaaa"),
	kHiliteColor("#ffd281"),
	kCurrentLineColor("#ffffcc"),
	kMarkedLineColor("#efff7f"),
	kPCLineColor = MColor("#cce5ff"),
	kBreakpointColor = MColor("#5ea50c"),
	kWhiteSpaceColor = MColor("#cf4c42");


class ExampleCanvas : public MCanvas
{
  public:
	ExampleCanvas();

	void Draw() override;
	void DrawLine(uint32_t inLineNr, MDevice &inDevice, MRect inLineRect);
};

ExampleCanvas::ExampleCanvas()
	: MCanvas("text-view", MRect{ 0, 0, 800, 800 })
{
}

void ExampleCanvas::Draw()
{
	MRect bounds = GetBounds();

	MDevice dev(this);
	dev.SetFont("Monospace 12");

	dev.EraseRect(bounds);

	auto lineHeight = dev.GetLineHeight();

	int32_t minLine = mBounds.y / lineHeight - 1;
	if (minLine < 0)
		minLine = 0;

	uint32_t maxLine = minLine + bounds.height / lineHeight + 2;

	for (uint32_t line = minLine; line <= maxLine; ++line)
	{
		MRect lineRect(0, line * lineHeight, bounds.width + mBounds.x, lineHeight);
		if (bounds.Intersects(lineRect))
			DrawLine(line, dev, lineRect);
	}
}

void ExampleCanvas::DrawLine(uint32_t inLineNr, MDevice &inDevice, MRect inLineRect)
{
	std::string text = "This is a line of text";
	// mDocument->GetStyledText(inLineNr, inDevice, text);

	inLineRect.x += 10;

	inDevice.SetFont("Monospace 12");
	inDevice.SetText(text);

	MDeviceContextSaver save(inDevice);

	int32_t y = inLineRect.y;
	int32_t x = inLineRect.x;

	// MSelection selection = mDocument->GetSelection();

	// bool marked = mDocument->IsLineMarked(inLineNr);
	// bool current = IsActive() and selection.HasCaretOnLine(*mDocument, inLineNr);

	bool marked = true, current = true;

	if (not(inDevice.IsPrinting()))
	{
		MDeviceContextSaver save(inDevice);

		bool fill = true;

		if (marked and current)
			inDevice.CreateAndUsePattern(kMarkedLineColor, kCurrentLineColor);
		else if (marked)
			inDevice.SetForeColor(kMarkedLineColor);
		else if (current)
			inDevice.SetForeColor(kCurrentLineColor);
		else
			fill = false;

		if (fill)
		{
			inDevice.FillRect(inLineRect);

			MRect r2(inLineRect);
			r2.x -= r2.height / 2;
			r2.width = r2.height;

			inDevice.FillEllipse(r2);
		}
	}

	// // Highlight selection
	// if (not inDevice.IsPrinting())
	// {
	// 	inDevice.Save();
	// 	MColor selectionColor;

	// 	if (IsActive() or mDrawForDragImage)
	// 		selectionColor = gHiliteColor;
	// 	else
	// 		selectionColor = gInactiveHiliteColor;

	// 	for (auto [b, e] : selection.GetColumnsForLine(*mDocument, inLineNr))
	// 	{
	// 		if (b == e) // just a caret
	// 			continue;

	// 		MRect r = inLineRect;
	// 		r.x += indent + b * mCharWidth;
	// 		r.width = (e - b) * mCharWidth;

	// 		inDevice.SetForeColor(selectionColor);
	// 		inDevice.FillRect(r);
	// 	}
	// 	inDevice.Restore();
	// }

	inDevice.RenderText(x, y);

	// if (mCaretVisible)
	// {
		int caretColumn = 0;
		// for (auto caretColumn : selection.GetCaretsForLine(*mDocument, inLineNr))
			inDevice.DrawCaret(x, y, caretColumn);
	// }
}

class ExampleWindow : public MDocWindow
{
  public:
	ExampleWindow()
		: MDocWindow("Example", MRect{ 0, 0, 400, 400 }, kMPostionDefault | kMShowMenubar)
		, cClose(this, "close", &ExampleWindow::Close, 'W', kControlKey | kShiftKey)
	{
		SetTitle("window-" + std::to_string(++s_nr));

		ExampleCanvas *canvas = new ExampleCanvas();
		canvas->SetLayout({ true, 0 });
		AddChild(canvas);
	}

	void Close()
	{
		MWindow::Close();
	}

	MCommand<void()> cClose;

	static int s_nr;
};

int ExampleWindow::s_nr;

// --------------------------------------------------------------------

class ExampleApp : public MDocApplication
{
  public:
	ExampleApp(MApplicationImpl *impl)
		: MDocApplication(impl)
		, cNew(this, "new", &ExampleApp::DoNew, 'N', kControlKey)
		, cQuit(this, "quit", &ExampleApp::DoQuit, 'Q', kControlKey)
	{
	}

	void Initialise() override
	{
		MApplication::Initialise();
		MMenuBar::Init("example-menu");
	}

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
