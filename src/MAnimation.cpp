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
#include "MAnimationImpl.hpp"

using namespace std;

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
	: mImpl(MAnimationManagerImpl::Create(this))
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
