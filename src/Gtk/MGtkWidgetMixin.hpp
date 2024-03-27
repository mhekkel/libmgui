/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Maarten L. Hekkelman
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "MGtkLib.hpp"
#include "MGtkCommandEmitter.hpp"

#include "MAlerts.hpp"
#include "MControls.hpp"
#include "MError.hpp"
#include "MView.hpp"

#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

template <class CallbackIn, typename Function>
struct MGtkCallbackOutHandler
{
};

template <class CallbackIn, typename Result, typename... Arguments>
struct MGtkCallbackOutHandler<CallbackIn, Result(Arguments...)>
{
	std::unique_ptr<CallbackIn> mHandler;
	GObject *mSendingGObject;

	static Result GCallback(GObject *inObject, Arguments... args, gpointer inData)
	{
		Result result{};

		try
		{
			MGtkCallbackOutHandler &handler = *reinterpret_cast<MGtkCallbackOutHandler *>(inData);

			if (handler.mHandler.get() != nullptr)
			{
				handler.mSendingGObject = inObject;
				result = handler.mHandler->DoCallback(args...);
			}
		}
		catch (...)
		{
			std::cerr << "caught exception in GCallback" << std::endl;
		}

		return result;
	}
};

// What the f*ck... optimising code nukes the default, somehow bool and int are not equal
template <class CallbackIn, typename... Arguments>
struct MGtkCallbackOutHandler<CallbackIn, bool(Arguments...)>
{
	std::unique_ptr<CallbackIn> mHandler;
	GObject *mSendingGObject;

	static gboolean GCallback(GObject *inObject, Arguments... args, gpointer inData)
	{
		gboolean result = FALSE;

		try
		{
			MGtkCallbackOutHandler &handler = *reinterpret_cast<MGtkCallbackOutHandler *>(inData);

			if (handler.mHandler.get() != nullptr)
			{
				handler.mSendingGObject = inObject;
				if (handler.mHandler->DoCallback(args...))
					result = TRUE;
			}
		}
		catch (...)
		{
			std::cerr << "caught exception in GCallback" << std::endl;
		}

		return result;
	}
};

template <class CallbackIn, typename... Arguments>
struct MGtkCallbackOutHandler<CallbackIn, void(Arguments...)>
{
	std::unique_ptr<CallbackIn> mHandler;
	GObject *mSendingGObject;

	static void GCallback(GObject *inObject, Arguments... args, gpointer inData)
	{
		try
		{
			MGtkCallbackOutHandler &handler = *reinterpret_cast<MGtkCallbackOutHandler *>(inData);

			if (handler.mHandler.get() != nullptr)
			{
				handler.mSendingGObject = inObject;
				handler.mHandler->DoCallback(args...);
			}
		}
		catch (...)
		{
			std::cerr << "caught exception in GCallback" << std::endl;
		}
	}
};

template <typename Function>
struct HandlerBase;

template <typename Result, typename... Arguments>
struct HandlerBase<Result(Arguments...)>
{
	virtual ~HandlerBase() {}
	virtual Result DoCallback(Arguments... args) = 0;
};

template <class Derived, class Owner, typename Function>
struct Handler;

//
//	Next is the Handler which derives from HandlerBase
//
template <class Derived, class Owner, typename Result, typename... Arguments>
struct Handler<Derived, Owner, Result(Arguments...)> : public HandlerBase<Result(Arguments...)>
{
	typedef Result (Owner::*Callback)(Arguments...);

	virtual Result DoCallback(Arguments... args)
	{
		Derived *self = static_cast<Derived *>(this);
		Owner *owner = self->mOwner;
		Callback func = self->mHandler;

		Result result = {};

		try
		{
			if (owner and func)
				result = (owner->*func)(args...);
		}
		catch (const std::exception &e)
		{
			DisplayError(e);
		}

		return result;
	}
};

template <class Derived, class Owner, typename... Arguments>
struct Handler<Derived, Owner, void(Arguments...)> : public HandlerBase<void(Arguments...)>
{
	typedef void (Owner::*Callback)(Arguments...);

	virtual void DoCallback(Arguments... args)
	{
		Derived *self = static_cast<Derived *>(this);
		Owner *owner = self->mOwner;
		Callback func = self->mHandler;

		try
		{
			(owner->*func)(args...);
		}
		catch (const std::exception &e)
		{
			DisplayError(e);
		}
	}
};

template <class C, typename Function>
struct MCallbackInHandler : public Handler<MCallbackInHandler<C, Function>, C, Function>
{
	typedef Handler<MCallbackInHandler, C, Function> base;
	typedef typename base::Callback CallbackProc;
	typedef C Owner;

	MCallbackInHandler(Owner *inOwner, CallbackProc inHandler)
		: mOwner(inOwner)
		, mHandler(inHandler)
	{
	}

	C *mOwner;
	CallbackProc mHandler;
};

template <typename Function>
struct MakeGtkCallbackHandler
{
	typedef MGtkCallbackOutHandler<
		HandlerBase<Function>,
		Function>
		type;
};

template <typename Function>
class MSlot : public MakeGtkCallbackHandler<Function>::type
{
	typedef typename MakeGtkCallbackHandler<Function>::type base_class;

  public:
	MSlot(const MSlot &) = delete;
	MSlot &operator=(const MSlot &) = delete;

	template <class C>
	MSlot(C *inOwner, typename MCallbackInHandler<C, Function>::CallbackProc inProc)
	{
		base_class *self = static_cast<base_class *>(this);
		typedef MCallbackInHandler<C, Function> Handler;

		self->mHandler.reset(new Handler(inOwner, inProc));
	}

	void Connect(GObject *inObject, const char *inSignalName)
	{
		THROW_IF_NIL(inObject);
		mID = g_signal_connect(inObject, inSignalName,
			G_CALLBACK(&base_class::GCallback), this);
	}

	void Connect(GtkWidget *inObject, const char *inSignalName)
	{
		Connect(G_OBJECT(inObject), inSignalName);
	}

	void Disconnect(GtkWidget *inObject)
	{
		if (mID > 0)
			g_signal_handler_disconnect(G_OBJECT(inObject), mID);
		mID = 0;
	}

	void Block(GtkWidget *inObject, const char *inSignalName)
	{
		THROW_IF_NIL(inObject);
		g_signal_handlers_block_by_func(G_OBJECT(inObject),
			(void *)G_CALLBACK(&base_class::GCallback), this);
	}

	void Unblock(GtkWidget *inObject, const char *inSignalName)
	{
		THROW_IF_NIL(inObject);
		g_signal_handlers_unblock_by_func(G_OBJECT(inObject),
			(void *)G_CALLBACK(&base_class::GCallback), this);
	}

	GObject *GetSourceGObject() const
	{
		return base_class::mSendingGObject;
	}

  private:
	ulong mID = 0;
};

// --------------------------------------------------------------------

enum class MEventMask
{
	None = 0,
	Focus = (1 << 0),
	GestureClick = (1 << 1),
	Key = (1 << 2),
	Pointer = (1 << 3),
	Scroll = (1 << 4),

	All = (Focus | GestureClick | Key | Pointer | Scroll)
};

constexpr MEventMask operator|(MEventMask a, MEventMask b)
{
	return static_cast<MEventMask>(int(a) | int(b));
}

constexpr bool operator&(MEventMask a, MEventMask b)
{
	return (int(a) & int(b)) != 0;
}

// --------------------------------------------------------------------

class MGtkWidgetMixin : public MGtkCommandEmitter
{
  public:
	MGtkWidgetMixin(const MGtkWidgetMixin &) = delete;
	MGtkWidgetMixin &operator=(const MGtkWidgetMixin &) = delete;

	MGtkWidgetMixin(MEventMask inEvents = MEventMask::None);
	virtual ~MGtkWidgetMixin();

	void SetEventMask(MEventMask inEvents)
	{
		mEvents = inEvents;
	}

	void RequestSize(int32_t inWidth, int32_t inHeight);

	bool IsActive() const;
	virtual void SetFocus();
	virtual void ReleaseFocus();
	virtual bool IsFocus() const;

	void CreateIMContext();

	operator GtkWidget *() { return mWidget; }

	GtkWidget *GetWidget() const { return mWidget; }
	void SetWidget(GtkWidget *inWidget);

	virtual void Append(MGtkWidgetMixin *inChild);

	GObject *GetActionMapObject() override
	{
		return G_OBJECT(GetWidget());
	}

	void AddShortcut(GtkShortcut *inShortcut)
	{
		if (mShortcutController == nullptr)
		{
			mShortcutController = gtk_shortcut_controller_new();
			gtk_widget_add_controller(GetWidget(), mShortcutController);
			
		}
		
		gtk_shortcut_controller_add_shortcut(
			GTK_SHORTCUT_CONTROLLER(mShortcutController), inShortcut);
	}

  protected:
	GtkWidget *mWidget;

	// --------------------------------------------------------------------
	// GtkWidget signals

	virtual void OnDestroy();
	virtual void OnDirectionChanged(GtkTextDirection previous_direction);
	virtual void OnHide();
	virtual bool OnKeynavFailed(GtkDirectionType direction);
	virtual void OnMap();
	virtual bool OnMnemonicActivate(gboolean group_cycling);
	virtual void OnMoveFocus(GtkDirectionType direction);
	virtual bool OnQueryTooltip(gint x, gint y, gboolean keyboard_mode, GtkTooltip *tooltip);
	virtual void OnRealize();
	virtual void OnShow();
	virtual void OnStateFlagsChanged(GtkStateFlags flags);
	virtual void OnUnmap();
	virtual void OnUnrealize();

	MSlot<void()> mDestroy;
	MSlot<void(GtkTextDirection)> mDirectionChanged;
	MSlot<void()> mHide;
	MSlot<bool(GtkDirectionType)> mKeynavFailed;
	MSlot<void()> mMap;
	MSlot<bool(gboolean)> mMnemonicActivate;
	MSlot<void(GtkDirectionType)> mMoveFocus;
	MSlot<bool(gint, gint, gboolean, GtkTooltip *)> mQueryTooltip;
	MSlot<void()> mRealize;
	MSlot<void()> mShow;
	MSlot<void(GtkStateFlags)> mStateFlagsChanged;
	MSlot<void()> mUnmap;
	MSlot<void()> mUnrealize;

	// --------------------------------------------------------------------
	// GtkIMContext signals

	virtual void OnCommit(char *text);
	virtual bool OnDeleteSurrounding(int offset, int n_chars);
	virtual void OnPreeditChanged();
	virtual void OnPreeditEnd();
	virtual void OnPreeditStart();
	virtual bool OnRetrieveSurrounding();

	MSlot<void(char *)> mCommit;
	MSlot<bool(int, int)> mDeleteSurrounding;
	MSlot<void()> mPreeditChanged;
	MSlot<void()> mPreeditEnd;
	MSlot<void()> mPreeditStart;
	MSlot<bool()> mRetrieveSurrounding;

	// --------------------------------------------------------------------
	// Event handling

	virtual void OnFocusEnter() {}
	virtual void OnFocusLeave() {}

	virtual void OnGestureClickPressed(double inX, double inY, gint inClickCount) {}
	virtual void OnGestureClickReleased(double inX, double inY, gint inClickCount) {}
	virtual void OnGestureClickStopped() {}

	virtual void OnPointerEnter(double inX, double inY) {}
	virtual void OnPointerMotion(double inX, double inY) {}
	virtual void OnPointerLeave() {}

	virtual bool OnKeyPressed(guint inKeyValue, guint inKeyCode, GdkModifierType inModifiers) { return false; }
	virtual void OnKeyReleased(guint inKeyValue, guint inKeyCode, GdkModifierType inModifiers) {}
	virtual void OnKeyModifiers(GdkModifierType inModifiers) {}

	virtual void OnDecelerate(double inVelX, double inVelY);
	virtual bool OnScroll(double inX, double inY);
	virtual void OnScrollBegin();
	virtual void OnScrollEnd();

	MSlot<void()> mFocusEnter;
	MSlot<void()> mFocusLeave;

	MSlot<void(double, double, gint)> mGestureClickPressed;
	MSlot<void(double, double, gint)> mGestureClickReleased;
	MSlot<void()> mGestureClickStopped;

	MSlot<void(double, double)> mPointerEnter;
	MSlot<void(double, double)> mPointerMotion;
	MSlot<void()> mPointerLeave;

	MSlot<bool(guint, guint, GdkModifierType)> mKeyPressed;
	MSlot<void(guint, guint, GdkModifierType)> mKeyReleased;
	MSlot<void(GdkModifierType)> mKeyModifiers;

	MSlot<void(double, double)> mDecelerate;
	MSlot<bool(double, double)> mScroll;
	MSlot<void()> mScrollBegin;
	MSlot<void()> mScrollEnd;

  protected:
	int32_t mRequestedWidth, mRequestedHeight;
	bool mAutoRepeat;

  private:
	GtkIMContext *mIMContext;
	bool mNextKeyPressIsAutoRepeat;
	MEventMask mEvents;

	std::optional<std::chrono::time_point<std::chrono::steady_clock>> mGainedFocusAt;

	GtkEventController *mShortcutController = nullptr;
};
