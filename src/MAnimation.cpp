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

#include "MAnimation.hpp"
#include "MApplication.hpp"
#include "MError.hpp"

#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <numeric>
#include <thread>
#include <utility>

// --------------------------------------------------------------------

class MAnimationVariableImpl
{
  public:
	MAnimationVariableImpl(double inValue, double inMin, double inMax)
		: mValue(inValue)
		, mMin(inMin)
		, mMax(inMax)
	{
	}

	void SetValue(double inValue)
	{
		if (inValue > mMax)
			inValue = mMax;
		if (inValue < mMin)
			inValue = mMin;
		mValue = inValue;
	}

	double GetValue() const { return mValue; }

  private:
	double mValue, mMin, mMax;
};

// --------------------------------------------------------------------

class MStoryboardImpl
{
  public:
	MStoryboardImpl() = default;

	void AddTransition(MAnimationVariable *inVariable,
		double inNewValue, std::chrono::system_clock::duration inDuration,
		const char *inTransitionName);

	void AddFinishedCallback(std::function<void()> cb)
	{
		mFinishedCB = cb;
	}

	// inTime is relative to the start of the story
	bool Update(std::chrono::system_clock::duration inTime);
	bool Done(std::chrono::system_clock::duration inTime);

	void Finish()
	{
		if (mFinishedCB)
			mFinishedCB();
	}

	struct MTransition
	{
		double mNewValue;
		std::chrono::system_clock::duration mDuration;
		std::string mTransitionName;
	};

	struct MVariableStory
	{
		MAnimationVariable *mVariable;
		double mStartValue;
		std::list<MTransition> mTransistions;
	};

	std::list<MVariableStory> mVariableStories;
	std::function<void()> mFinishedCB;
};

void MStoryboardImpl::AddTransition(MAnimationVariable *inVariable,
	double inNewValue, std::chrono::system_clock::duration inDuration, const char *inTransitionName)
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

bool MStoryboardImpl::Update(std::chrono::system_clock::duration inTime)
{
	bool result = false;

	for (auto vs : mVariableStories)
	{
		double v = vs.mStartValue;
		auto time = inTime;

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
			auto dv = t.mNewValue - v;
			if (t.mDuration > std::chrono::system_clock::duration{})
				dv = (dv * time) / t.mDuration;
			v += dv;
			break;
		}

		if (v != vs.mVariable->GetValue())
		{
			result = true;
			vs.mVariable->GetImpl()->SetValue(v);
		}
	}

	return result;
}

bool MStoryboardImpl::Done(std::chrono::system_clock::duration inTime)
{
	bool result = true;

	for (auto vs : mVariableStories)
	{
		auto totalDuration = std::accumulate(vs.mTransistions.begin(), vs.mTransistions.end(), std::chrono::system_clock::duration{},
			[](std::chrono::system_clock::duration time, const MTransition &ts) -> std::chrono::system_clock::duration
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

class MAnimationManagerImpl
{
  public:
	MAnimationManagerImpl(MAnimationManager *inManager)
		: mAnimationManager(inManager)
		, mDone(false)
		, mThread(std::bind(&MAnimationManagerImpl::Run, this))
	{
	}

	~MAnimationManagerImpl()
	{
		if (not mDone)
			Stop();
	}

	bool Update() { return false; }

	void Stop()
	{
		if (mDone)
			return;

		if (mIdleTag != 0)
		{
			// g_source_remove(mIdleTag);
			mIdleCondition.notify_one();
		}

		try
		{
			std::unique_lock<std::mutex> lock(mMutex);

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

	MAnimationVariable *CreateVariable(double inValue, double inMin, double inMax)
	{
		return new MAnimationVariable(new MAnimationVariableImpl(inValue, inMin, inMax));
	}

	MStoryboard *CreateStoryboard()
	{
		return new MStoryboard(new MStoryboardImpl());
	}

	void Schedule(MStoryboard *inStoryboard)
	{
		try
		{
			std::unique_lock<std::mutex> lock(mMutex);

			std::shared_ptr<MStoryboard> sbptr(inStoryboard);
			MScheduledStoryboard sb = { std::chrono::system_clock::now(), sbptr };
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
		std::chrono::system_clock::time_point mStartTime;
		std::shared_ptr<MStoryboard> mStoryboard;
	};

	void Idle();

	MAnimationManager *mAnimationManager;
	std::list<MScheduledStoryboard> mStoryboards;
	std::mutex mMutex;
	std::condition_variable mCondition, mIdleCondition;
	int mIdleTag = 0;
	bool mDone;
	std::thread mThread;
};

void MAnimationManagerImpl::Run()
{
	using namespace std::chrono_literals;

	for (;;)
	{
		try
		{
			std::unique_lock<std::mutex> lock(mMutex);

			if (mDone)
				break;

			if (mStoryboards.empty())
			{
				mCondition.wait_for(lock, 1s);
				continue;
			}

			bool update = false;

			auto now = std::chrono::system_clock::now();

			for (auto storyboard : mStoryboards)
			{
				MStoryboardImpl *storyboardImpl = static_cast<MStoryboardImpl *>(storyboard.mStoryboard->GetImpl());
				update = storyboardImpl->Update(now - storyboard.mStartTime) or update;
			}

			if (update)
			{
				mIdleTag = 1;
				gApp->ExecuteAsync(std::bind(&MAnimationManagerImpl::Idle, this));
				mIdleCondition.wait(lock);
			}

			mStoryboards.erase(std::remove_if(mStoryboards.begin(), mStoryboards.end(),
								   [now](MScheduledStoryboard &storyboard) -> bool
								   {
									   MStoryboardImpl *storyboardImpl = static_cast<MStoryboardImpl *>(storyboard.mStoryboard->GetImpl());
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

void MAnimationManagerImpl::Idle()
{
	if (std::exchange(mIdleTag, 0))
	{
		try
		{
			std::unique_lock<std::mutex> lock(mMutex);
			mAnimationManager->eAnimate();
		}
		catch (const std::exception &e)
		{
			PRINT(("Exception: %s", e.what()));
		}
		catch (...)
		{
			PRINT(("Exception"));
		}

		mIdleCondition.notify_one();
	}
}

// --------------------------------------------------------------------

MAnimationVariable::MAnimationVariable(MAnimationVariableImpl *inImpl)
	: mImpl(inImpl)
{
}

MAnimationVariable::~MAnimationVariable()
{
	delete mImpl;
}

double MAnimationVariable::GetValue() const
{
	return mImpl->GetValue();
}

// --------------------------------------------------------------------

MStoryboard::MStoryboard(MStoryboardImpl *inImpl)
	: mImpl(inImpl)
{
}

MStoryboard::~MStoryboard()
{
	delete mImpl;
}

void MStoryboard::AddTransition(MAnimationVariable *inVariable,
	double inNewValue, std::chrono::system_clock::duration inDuration, const char *inTransitionName)
{
	mImpl->AddTransition(inVariable, inNewValue, inDuration, inTransitionName);
}

void MStoryboard::AddFinishedCallback(std::function<void()> cb)
{
	mImpl->AddFinishedCallback(cb);
}

// --------------------------------------------------------------------

MAnimationManager::MAnimationManager()
	: mImpl(new MAnimationManagerImpl(this))
{
}

MAnimationManager::~MAnimationManager()
{
	delete mImpl;
}

bool MAnimationManager::Update()
{
	return mImpl->Update();
}

void MAnimationManager::Stop()
{
	mImpl->Stop();
}

MAnimationVariable *MAnimationManager::CreateVariable(double inValue, double inMin, double inMax)
{
	return mImpl->CreateVariable(inValue, inMin, inMax);
}

MStoryboard *MAnimationManager::CreateStoryboard()
{
	return mImpl->CreateStoryboard();
}

void MAnimationManager::Schedule(MStoryboard *inStoryboard)
{
	mImpl->Schedule(inStoryboard);
}
