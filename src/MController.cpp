//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MController.hpp"

#include "MAlerts.hpp"
#include "MDocWindow.hpp"
#include "MDocument.hpp"
#include "MError.hpp"
#include "MPreferences.hpp"

#include <cassert>

MController::MController(MDocWindow *inDocWindow)
	: mDocument(nullptr)
	, mDocWindow(inDocWindow)
{
}

MController::~MController()
{
	assert(mDocument == nullptr);
}

void MController::SetDocument(MDocument *inDocument)
{
	assert(mDocWindow);

	if (inDocument != mDocument)
	{
		if (mDocument != nullptr)
		{
			try
			{
				if (mDocument->IsSpecified() and MPrefs::GetInteger("save state", 1))
					mDocWindow->SaveState();
			}
			catch (...)
			{
			}

			mDocWindow->RemoveRoutes(mDocument);
			mDocument->RemoveController(this);
		}

		mDocument = inDocument;

		if (mDocument != nullptr)
		{
			mDocument->AddController(this);
			mDocWindow->AddRoutes(mDocument);
		}

		eDocumentChanged(mDocument);
	}
}

bool MController::TryCloseDocument(MCloseReason inAction)
{
	bool result = true;

	if (mDocument != nullptr)
	{
		if (not mDocument->IsModified())
			SetDocument(nullptr);
		else
		{
			result = false;
			std::string name;

			if (mDocument->IsSpecified())
				name = mDocument->GetFile().filename();
			else
				name = mDocWindow->GetTitle();

			MSaverMixin::TryCloseDocument(inAction, name, mDocWindow);
		}
	}

	return result;
}

bool MController::TryCloseController(MCloseReason inAction)
{
	bool result = true;

	if (mDocument != nullptr)
	{
		if (mDocument->CountControllers() > 1 or not mDocument->IsModified())
			SetDocument(nullptr);
		else
		{
			TryCloseDocument(inAction);
			mCloseOnNavTerminate = true;
			result = false;
		}
	}

	return result;
}

void MController::SaveDocumentAs()
{
	std::string name;

	if (mDocument->IsSpecified())
		name = mDocument->GetFile().filename();
	else
		name = mDocWindow->GetTitle();

	MSaverMixin::SaveDocumentAs(mDocWindow, name);
}

void MController::TryDiscardChanges()
{
	if (mDocument == nullptr)
		return;

	MSaverMixin::TryDiscardChanges(mDocument->GetFile().filename(), mDocWindow);
}

bool MController::SaveDocument()
{
	bool result = true;

	try
	{
		if (mDocument != nullptr)
		{
			if (mDocument->IsSpecified() and not mDocument->IsReadOnly())
				result = mDocument->DoSave();
			else
			{
				result = false;
				SaveDocumentAs();
			}
		}
	}
	catch (const std::exception &inErr)
	{
		DisplayError(inErr);
		result = false;
	}

	return result;
}

void MController::RevertDocument()
{
	mDocument->RevertDocument();
}

bool MController::DoSaveAs(const std::filesystem::path &inPath)
{
	return mDocument->DoSaveAs(inPath);
}

void MController::CloseAfterNavigationDialog()
{
	SetDocument(nullptr);
}

void MController::Print()
{
}
