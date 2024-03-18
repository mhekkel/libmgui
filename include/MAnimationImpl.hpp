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

#include "MAnimation.hpp"

// --------------------------------------------------------------------

class MAnimationManagerImpl
{
  public:
	virtual ~MAnimationManagerImpl() {}

	static MAnimationManagerImpl *Create(MAnimationManager *inManager);

	virtual bool Update() = 0;
	virtual void Stop() {}

	virtual MAnimationVariable *CreateVariable(double inValue, double inMin, double inMax) = 0;
	virtual MStoryboard *CreateStoryboard() = 0;
	virtual void Schedule(MStoryboard *inStoryboard) = 0;

  protected:
	MAnimationManagerImpl() {}
};

// --------------------------------------------------------------------

class MAnimationVariableImpl
{
  public:
	virtual ~MAnimationVariableImpl() {}

	virtual double GetValue() const = 0;

  protected:
	MAnimationVariableImpl() {}
};

// --------------------------------------------------------------------

class MStoryboardImpl
{
  public:
	virtual ~MStoryboardImpl() {}

	virtual void AddTransition(MAnimationVariable *inVariable,
		double inNewValue, double inDuration,
		const char *inTransitionName) = 0;

	virtual void AddFinishedCallback(std::function<void()> cb) = 0;

  protected:
	MStoryboardImpl() {}
};

#if 0
// --------------------------------------------------------------------
//	A fall back implementation using threads and such

class MFallBackAnimationManagerImpl : public MAnimationManagerImpl
{

};

#endif // 0
