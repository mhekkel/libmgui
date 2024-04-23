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

/*	$Id: MP2PEvents.h 119 2007-04-25 08:26:21Z maarten $

    Copyright Hekkelman Programmatuur b.v.
    Maarten Hekkelman

    Created: Friday April 28 2000 15:57:00
    Completely rewritten in July 2005

    This is an implementation of point to point events. You can deliver messages from one
    object to another using the EventIn and EventOut objects. One of the huge advantages
    of using p2p events instead of simply calling another objects's method is that dangling
    pointers are avoided, the event objects know how to detach from each other.

    For now this implementation allows the passing of 0 to 4 arguments. If needed more
    can be added very easily.

    Usage:

    struct foo
    {
        MEventOut<void(int)>	eBeep;
    };

    struct bar
    {
        MEventIn<void(int)>		eBeep;

        void					Beep(int inBeeps);

                                bar();
    };

    bar::bar()
        : eBeep(this, &bar::Beep)
    {
    }

    void bar::Beep(int inBeeps)
    {
        cout << "beep " << inBeeps << " times" << endl;
    }

    ...

    foo f;
    bar b;

    AddRoute(f.eBeep, b.eBeep);

    f.eBeep(2);

*/

#pragma once

#include <algorithm>
#include <functional>
#include <list>

// forward declarations of our event types

template <typename Function>
class MEventIn;
template <typename Function>
class MEventOut;

// the actual MEventIn class is a specialization of the MEventIn templated class

template <typename... Args>
class MEventIn<void(Args...)>
{
	typedef MEventOut<void(Args...)> MEventOutType;
	typedef std::list<MEventOutType *> MEventOutList;

  public:
	template <class C, typename M = void (C::*)(Args...)>
	MEventIn(C *inOwner, M inHandler)
	{
		mHandler = [inOwner, inHandler](Args... args)
		{ (inOwner->*inHandler)(args...); };
	}

	MEventIn(std::function<void(Args...)> inHandler)
		: mHandler(inHandler)
	{
	}

	MEventIn(MEventIn &&rhs)
		: mHandler(std::move(rhs.mHandler))
	{
	}

	~MEventIn()
	{
		mHandler = {};

		MEventOutList events;
		std::swap(events, mOutEvents);
		for (auto event : events)
			event->RemoveEventIn(this);
	}

	void AddEventOut(MEventOutType *inEventOut)
	{
		if (std::find(mOutEvents.begin(), mOutEvents.end(), inEventOut) == mOutEvents.end())
			mOutEvents.push_back(inEventOut);
	}

	void RemoveEventOut(MEventOutType *inEventOut)
	{
		mOutEvents.erase(std::remove(mOutEvents.begin(), mOutEvents.end(), inEventOut), mOutEvents.end());
	}

	void Handle(Args... args)
	{
		if (mHandler)
			mHandler(args...);
	}

  private:
	MEventIn(const MEventIn &);
	MEventIn &operator=(const MEventIn &);

	std::function<void(Args...)> mHandler;
	MEventOutList mOutEvents;
};

// and the MEventOut class

template <typename... Args>
class MEventOut<void(Args...)>
{
	typedef MEventIn<void(Args...)> MEventInType;
	typedef std::list<MEventInType *> MEventInList;

  public:
	MEventOut() {}
	~MEventOut()
	{
		for (auto event : mInEvents)
			event->RemoveEventOut(this);
	}

	void operator()(Args... args)
	{
		// copy the list before calling the events, avoids stale event handlers
		MEventInList events = mInEvents;
		for (auto event : events)
			event->Handle(args...);
	}

	void AddEventIn(MEventInType *inEventIn)
	{
		if (std::find(mInEvents.begin(), mInEvents.end(), inEventIn) == mInEvents.end())
		{
			inEventIn->AddEventOut(this);
			mInEvents.push_back(inEventIn);
		}
	}

	void RemoveEventIn(MEventInType *inEventIn)
	{
		inEventIn->RemoveEventOut(this);
		mInEvents.erase(std::remove(mInEvents.begin(), mInEvents.end(), inEventIn), mInEvents.end());
	}

	void operator+=(std::function<void(Args...)> func)
	{
		mInHandlers.push_back(MEventInType(func));
		mInEvents.push_back(&mInHandlers.back());
	}

  private:
	std::list<MEventInType> mInHandlers;
	MEventInList mInEvents;
};

// Use these functions to create and delete routes.

template <typename Function>
void AddRoute(MEventIn<Function> &inEventIn, MEventOut<Function> &inEventOut)
{
	inEventOut.AddEventIn(&inEventIn);
}

template <typename Function>
void AddRoute(MEventOut<Function> &inEventOut, MEventIn<Function> &inEventIn)
{
	inEventOut.AddEventIn(&inEventIn);
}

template <class Function>
void RemoveRoute(MEventIn<Function> &inEventIn, MEventOut<Function> &inEventOut)
{
	inEventOut.RemoveEventIn(&inEventIn);
}

template <class Function>
void RemoveRoute(MEventOut<Function> &inEventOut, MEventIn<Function> &inEventIn)
{
	inEventOut.RemoveEventIn(&inEventIn);
}
