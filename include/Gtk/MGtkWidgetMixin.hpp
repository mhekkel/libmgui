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

#include "MAlerts.hpp"
#include "MError.hpp"
#include "MView.hpp"

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

class MHandler;

typedef std::vector<std::pair<GObject *, std::string>> MSignalHandlerArray;

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

class MGtkWidgetMixin
{
  public:
	MGtkWidgetMixin(const MGtkWidgetMixin &) = delete;
	MGtkWidgetMixin &operator=(const MGtkWidgetMixin &) = delete;

	MGtkWidgetMixin();
	//	MGtkWidgetMixin(GtkWidget* inWidget);
	virtual ~MGtkWidgetMixin();

	void RequestSize(int32_t inWidth, int32_t inHeight);

	bool IsActive() const;
	virtual void SetFocus();
	virtual void ReleaseFocus();
	virtual bool IsFocus() const;

	void CreateIMContext();

	operator GtkWidget *() { return mWidget; }

	GtkWidget *GetWidget() const { return mWidget; }
	void SetWidget(GtkWidget *inWidget);

	virtual void Append(MGtkWidgetMixin *inChild, MControlPacking inPacking,
		bool inExpand, bool inFill, uint32_t inPadding);

  protected:
	virtual bool OnDestroy();
	virtual void OnShow();
	virtual bool OnFocusInEvent(GdkEvent *inEvent);
	virtual bool OnFocusOutEvent(GdkEvent *inEvent);
	virtual bool OnButtonPressEvent(GdkEvent *inEvent);
	virtual bool OnMotionNotifyEvent(GdkEvent *inEvent);
	virtual bool OnLeaveNotifyEvent(GdkEvent *inEvent);
	virtual bool OnButtonReleaseEvent(GdkEvent *inEvent);
	virtual bool OnKeyPressEvent(GdkEvent *inEvent);
	virtual bool OnKeyReleaseEvent(GdkEvent *inEvent);
	virtual bool OnConfigureEvent(GdkEvent *inEvent);
	virtual bool OnScrollEvent(GdkEvent *inEvent);
	virtual bool OnRealize();
	virtual bool OnDrawEvent(cairo_t *inCairo);
	virtual void OnPopupMenu();

	// Mouse support

	virtual bool OnMouseDown(int32_t inX, int32_t inY, uint32_t inButtonNr, uint32_t inClickCount, uint32_t inModifiers);
	virtual bool OnMouseMove(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual bool OnMouseUp(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual bool OnMouseExit();

	// Drag and Drop support

	// void SetupDragAndDrop(const GtkTargetEntry inTargets[], uint32_t inTargetCount);
	// void DragBegin(const GtkTargetEntry inTargets[], uint32_t inTargetCount, GdkEventMotion *inEvent);
	// virtual void DragEnter();
	// virtual bool DragWithin(int32_t inX, int32_t inY);
	// virtual void DragLeave();
	// virtual bool DragAccept(bool inMove, int32_t inX, int32_t inY, const char *inData, uint32_t inLength, uint32_t inType);
	// virtual void DragSendData(std::string &outData);
	// virtual void DragDeleteData();
	// virtual void OnDragDataReceived(GdkDragContext *inDragContext, gint inX, gint inY, GtkSelectionData *inData, guint inInfo, guint inTime);
	// virtual bool OnDragMotion(GdkDragContext *inDragContext, gint inX, gint inY, guint inTime);
	// virtual void OnDragLeave(GdkDragContext *inDragContext, guint inTime);
	// virtual void OnDragDataDelete(GdkDragContext *inDragContext);
	// virtual void OnDragDataGet(GdkDragContext *inDragContext, GtkSelectionData *inData, guint inInfo, guint inTime);
	// bool IsWithinDrag() const { return mDragWithin; }
	// //	virtual void	DrawDragImage(GdkPixmap*& outPixmap, int32_t& outX, int32_t& outY)	{  }

	GtkWidget *mWidget;

	MSlot<bool()> mOnDestroy;
	MSlot<void()> mOnShow;

	MSlot<bool(GdkEvent *)> mFocusInEvent;
	MSlot<bool(GdkEvent *)> mFocusOutEvent;
	MSlot<bool(GdkEvent *)> mButtonPressEvent;
	MSlot<bool(GdkEvent *)> mMotionNotifyEvent;
	MSlot<bool(GdkEvent *)> mLeaveNotifyEvent;
	MSlot<bool(GdkEvent *)> mButtonReleaseEvent;
	MSlot<bool(GdkEvent *)> mKeyPressEvent;
	MSlot<bool(GdkEvent *)> mKeyReleaseEvent;
	MSlot<bool(GdkEvent *)> mConfigureEvent;
	MSlot<bool(GdkEvent *)> mScrollEvent;
	MSlot<bool()> mRealize;
	MSlot<bool(cairo_t *)> mDrawEvent;
	MSlot<void()> mPopupMenu;

	// MSlot<void(GdkDragContext *,
	// 	gint,
	// 	gint,
	// 	GtkSelectionData *,
	// 	guint,
	// 	guint)>
	// 	mDragDataReceived;

	// MSlot<bool(GdkDragContext *,
	// 	gint,
	// 	gint,
	// 	guint)>
	// 	mDragMotion;

	// MSlot<void(GdkDragContext *,
	// 	guint)>
	// 	mDragLeave;

	// MSlot<void(GdkDragContext *)> mDragDataDelete;

	// MSlot<void(GdkDragContext *,
	// 	GtkSelectionData *,
	// 	guint,
	// 	guint)>
	// 	mDragDataGet;

	// IMContext support

	virtual bool OnCommit(gchar *inText);
	virtual bool OnDeleteSurrounding(gint inStart, gint inLength);
	virtual bool OnPreeditChanged();
	virtual bool OnPreeditEnd();
	virtual bool OnPreeditStart();
	virtual bool OnRetrieveSurrounding();
	virtual bool OnGrabBroken(GdkEvent *);

	MSlot<bool(gchar *)> mOnCommit;
	MSlot<bool(gint, gint)> mOnDeleteSurrounding;
	MSlot<bool()> mOnPreeditChanged;
	MSlot<bool()> mOnPreeditStart;
	MSlot<bool()> mOnPreeditEnd;
	MSlot<bool()> mOnRetrieveSurrounding;
	MSlot<bool(GdkEvent *)> mOnGrabBroken;

  protected:
	int32_t mRequestedWidth, mRequestedHeight;
	bool mAutoRepeat;

  private:
	bool mDragWithin;
	GtkIMContext *mIMContext;
	bool mNextKeyPressIsAutoRepeat;
	std::chrono::time_point<std::chrono::steady_clock> mGainedFocusAt;
};
