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

#include "MAnimationImpl.hpp"
#include "MAnimation.hpp"
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

class MFallBackAnimationVariableImpl : public MAnimationVariableImpl
{
  public:
	MFallBackAnimationVariableImpl(double inValue, double inMin, double inMax)
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

class MFallBackStoryboardImpl : public MStoryboardImpl
{
  public:
	MFallBackStoryboardImpl() {}
	virtual void AddTransition(MAnimationVariable *inVariable,
		double inNewValue, double inDuration,
		const char *inTransitionName);

	// inTime is relative to the start of the story
	bool Update(double inTime);
	bool Done(double inTime);

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
};

void MFallBackStoryboardImpl::AddTransition(MAnimationVariable *inVariable,
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

bool MFallBackStoryboardImpl::Update(double inTime)
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
			static_cast<MFallBackAnimationVariableImpl *>(vs.mVariable->GetImpl())->SetValue(v);
		}
	}

	return result;
}

bool MFallBackStoryboardImpl::Done(double inTime)
{
	bool result = true;

	for (auto vs : mVariableStories)
	{
		double totalDuration = std::accumulate(vs.mTransistions.begin(), vs.mTransistions.end(), 0.0,
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

class MFallBackAnimationManagerImpl : public MAnimationManagerImpl
{
  public:
	MFallBackAnimationManagerImpl(MAnimationManager *inManager)
		: mAnimationManager(inManager)
		, mDone(false)
		, mThread(bind(&MFallBackAnimationManagerImpl::Run, this))
	{
	}

	~MFallBackAnimationManagerImpl()
	{
		if (not mDone)
			Stop();
	}

	virtual bool Update() { return false; }
	virtual void Stop()
	{
		try
		{
			unique_lock<mutex> lock(mMutex);

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
		return new MAnimationVariable(new MFallBackAnimationVariableImpl(inValue, inMin, inMax));
	}

	virtual MStoryboard *CreateStoryboard()
	{
		return new MStoryboard(new MFallBackStoryboardImpl());
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

	struct MScheduledStoryboard
	{
		double mStartTime;
		shared_ptr<MStoryboard> mStoryboard;
	};

	MAnimationManager *mAnimationManager;
	list<MScheduledStoryboard> mStoryboards;
	mutex mMutex;
	condition_variable mCondition;
	bool mDone;
	thread mThread;
};

void MFallBackAnimationManagerImpl::Run()
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
				mCondition.wait(lock);
				continue;
			}

			bool update = false;

			double now = GetLocalTime();

			for (auto storyboard : mStoryboards)
			{
				MFallBackStoryboardImpl *storyboardImpl = static_cast<MFallBackStoryboardImpl *>(storyboard.mStoryboard->GetImpl());
				update = storyboardImpl->Update(now - storyboard.mStartTime) or update;
			}

			if (update)
			{
				// gdk_threads_enter();
				// try
				// {
				// 	mAnimationManager->eAnimate();
				// }
				// catch (exception& e)
				// {
				// 	PRINT(("Exception: %s", e.what()));
				// }
				// catch (...)
				// {
				// 	PRINT(("Exception"));
				// }

				// gdk_threads_leave();
			}

			mStoryboards.erase(std::remove_if(mStoryboards.begin(), mStoryboards.end(),
								   [now](MScheduledStoryboard &storyboard) -> bool
								   {
									   MFallBackStoryboardImpl *storyboardImpl = static_cast<MFallBackStoryboardImpl *>(storyboard.mStoryboard->GetImpl());
									   return storyboardImpl->Done(now - storyboard.mStartTime);
								   }),
				mStoryboards.end());
		}
		catch (...)
		{
		}

		std::this_thread::sleep_for(0.05s);
	}
}

// // --------------------------------------------------------------------

// MAnimationManagerImpl* MAnimationManagerImpl::Create(MAnimationManager* inManager)
// {
// //	return new MGtkAnimationManagerImpl(inManager);
// 	return new MWinAnimationManagerImpl(inManager);
// }
