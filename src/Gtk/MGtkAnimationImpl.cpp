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

#include "Gtk/MGtkLib.hpp"

#include "MAnimation.hpp"
#include "MAnimationImpl.hpp"
#include "MApplication.hpp"
#include "MError.hpp"
#include "MUtils.hpp"

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <numeric>
#include <thread>

using namespace std;

// --------------------------------------------------------------------

class MGtkAnimationVariableImpl : public MAnimationVariableImpl
{
  public:
	MGtkAnimationVariableImpl(double inValue, double inMin, double inMax)
		: mValue(inValue)
		, mMin(inMin)
		, mMax(inMax)
	{
	}

	virtual void SetValue(double inValue)
	{
		if (inValue > mMax)
			inValue = mMax;
		if (inValue < mMin)
			inValue = mMin;
		mValue = inValue;
	}

	virtual double GetValue() const { return mValue; }

  private:
	double mValue, mMin, mMax;
};

// --------------------------------------------------------------------

class MGtkStoryboardImpl : public MStoryboardImpl
{
  public:
	MGtkStoryboardImpl() {}
	virtual void AddTransition(MAnimationVariable *inVariable,
		double inNewValue, double inDuration,
		const char *inTransitionName);

	virtual void AddFinishedCallback(std::function<void()> cb)
	{
		mFinishedCB = cb;
	}

	// inTime is relative to the start of the story
	bool Update(double inTime);
	bool Done(double inTime);

	void Finish()
	{
		if (mFinishedCB)
			mFinishedCB();
	}

	struct MTransition
	{
		double mNewValue;
		double mDuration;
		string mTransitionName;
	};

	struct MVariableStory
	{
		MAnimationVariable *mVariable;
		double mStartValue;
		list<MTransition> mTransistions;
	};

	list<MVariableStory> mVariableStories;
	std::function<void()> mFinishedCB;
};

void MGtkStoryboardImpl::AddTransition(MAnimationVariable *inVariable,
	double inNewValue, double inDuration, const char *inTransitionName)
{
	auto s = find_if(mVariableStories.begin(), mVariableStories.end(),
		[inVariable](MVariableStory &st) -> bool
		{ return st.mVariable == inVariable; });

	if (s == mVariableStories.end())
	{
		MVariableStory st = { inVariable, inVariable->GetValue(), {} };
		mVariableStories.push_back(st);
		s = prev(mVariableStories.end());
	}

	assert(s != mVariableStories.end());

	MTransition t = { inNewValue, inDuration, inTransitionName };
	s->mTransistions.push_back(t);
}

bool MGtkStoryboardImpl::Update(double inTime)
{
	bool result = false;

	for (auto vs : mVariableStories)
	{
		double v = vs.mStartValue;
		double time = inTime;

		for (auto t : vs.mTransistions)
		{
			if (t.mDuration < time)
			{
				v = t.mNewValue;
				time -= t.mDuration;
				continue;
			}

			// alleen nog maar lineair
			assert(t.mTransitionName == "acceleration-decelleration");
			double dv = t.mNewValue - v;
			if (t.mDuration > 0)
				dv *= time / t.mDuration;
			v += dv;
			break;
		}

		if (v != vs.mVariable->GetValue())
		{
			result = true;
			static_cast<MGtkAnimationVariableImpl *>(vs.mVariable->GetImpl())->SetValue(v);
		}
	}

	return result;
}

bool MGtkStoryboardImpl::Done(double inTime)
{
	bool result = true;

	for (auto vs : mVariableStories)
	{
		double totalDuration = accumulate(vs.mTransistions.begin(), vs.mTransistions.end(), 0.0,
			[](double time, const MTransition &ts) -> double
			{ return time + ts.mDuration; });

		if (totalDuration > inTime)
		{
			result = false;
			break;
		}
	}

	return result;
}

// --------------------------------------------------------------------

class MGtkAnimationManagerImpl : public MAnimationManagerImpl
{
  public:
	MGtkAnimationManagerImpl(MAnimationManager *inManager)
		: mAnimationManager(inManager)
		, mDone(false)
		, mThread(bind(&MGtkAnimationManagerImpl::Run, this))
	{
	}

	~MGtkAnimationManagerImpl()
	{
		if (not mDone)
			Stop();
	}

	virtual bool Update() { return false; }
	virtual void Stop()
	{
		if (mDone)
			return;

		try
		{
			unique_lock<mutex> lock(mMutex);

			if (mIdleTag != 0)
			{
				g_source_remove(mIdleTag);
				mIdleCondition.notify_one();
			}

			mStoryboards.clear();
			mDone = true;

			mCondition.notify_one();
			lock.unlock();

			if (mThread.joinable())
				mThread.join();
		}
		catch (...)
		{
		}
	}

	virtual MAnimationVariable *CreateVariable(double inValue, double inMin, double inMax)
	{
		return new MAnimationVariable(new MGtkAnimationVariableImpl(inValue, inMin, inMax));
	}

	virtual MStoryboard *CreateStoryboard()
	{
		return new MStoryboard(new MGtkStoryboardImpl());
	}

	virtual void Schedule(MStoryboard *inStoryboard)
	{
		try
		{
			unique_lock<mutex> lock(mMutex);

			shared_ptr<MStoryboard> sbptr(inStoryboard);
			MScheduledStoryboard sb = { GetLocalTime(), sbptr };
			mStoryboards.push_back(sb);

			mCondition.notify_one();
		}
		catch (...)
		{
		}
	}

	void Run();

	static gboolean IdleCallback(gpointer data);

	struct MScheduledStoryboard
	{
		double mStartTime;
		shared_ptr<MStoryboard> mStoryboard;
	};

	MAnimationManager *mAnimationManager;
	list<MScheduledStoryboard> mStoryboards;
	mutex mMutex;
	condition_variable mCondition, mIdleCondition;
	bool mDone;
	thread mThread;
	guint mIdleTag = 0;
};

void MGtkAnimationManagerImpl::Run()
{
	for (;;)
	{
		try
		{
			unique_lock<mutex> lock(mMutex);

			if (mDone)
				break;

			if (mStoryboards.empty())
			{
				mCondition.wait_for(lock, 1s);
				continue;
			}

			bool update = false;

			double now = GetLocalTime();

			for (auto storyboard : mStoryboards)
			{
				MGtkStoryboardImpl *storyboardImpl = static_cast<MGtkStoryboardImpl *>(storyboard.mStoryboard->GetImpl());
				update = storyboardImpl->Update(now - storyboard.mStartTime) or update;
			}

			if (update)
			{
				mIdleTag = gdk_threads_add_idle(&MGtkAnimationManagerImpl::IdleCallback, this);
				mIdleCondition.wait(lock);
			}

			mStoryboards.erase(remove_if(mStoryboards.begin(), mStoryboards.end(),
								   [now](MScheduledStoryboard &storyboard) -> bool
								   {
									   MGtkStoryboardImpl *storyboardImpl = static_cast<MGtkStoryboardImpl *>(storyboard.mStoryboard->GetImpl());

									   bool result = storyboardImpl->Done(now - storyboard.mStartTime);

									   if (result)
										   storyboardImpl->Finish();

									   return result;
								   }),
				mStoryboards.end());
		}
		catch (...)
		{
		}

		this_thread::sleep_for(0.05s);
	}
}

gboolean MGtkAnimationManagerImpl::IdleCallback(gpointer data)
{
	MGtkAnimationManagerImpl *self = reinterpret_cast<MGtkAnimationManagerImpl *>(data);

	self->mIdleTag = 0;

	try
	{
		unique_lock<mutex> lock(self->mMutex);
		self->mAnimationManager->eAnimate();
	}
	catch (exception &e)
	{
		PRINT(("Exception: %s", e.what()));
	}
	catch (...)
	{
		PRINT(("Exception"));
	}

	self->mIdleCondition.notify_one();

	return false;
}

// --------------------------------------------------------------------

MAnimationManagerImpl *MAnimationManagerImpl::Create(MAnimationManager *inManager)
{
	return new MGtkAnimationManagerImpl(inManager);
}
