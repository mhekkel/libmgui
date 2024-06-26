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

#include <iostream>
#include <cstdint>

typedef char32_t unicode;

struct MRect
{
	int32_t x = 0, y = 0;
	int32_t width = 0, height = 0;

	MRect() {}

	MRect(int32_t inX, int32_t inY, int32_t inWidth, int32_t inHeight)
		: x(inX)
		, y(inY)
		, width(inWidth)
		, height(inHeight)
	{
	}

	MRect(const MRect &) = default;
	MRect &operator=(const MRect &) = default;

	bool Intersects(const MRect &inRHS) const

	{
		return x < inRHS.x + inRHS.width and
		       x + width > inRHS.x and
		       y < inRHS.y + inRHS.height and
		       y + height > inRHS.y;
	}

	bool ContainsPoint(int32_t inX, int32_t inY) const
	{
		return x <= inX and x + width > inX and
		       y <= inY and y + height > inY;
	}

	void PinPoint(int32_t &ioX, int32_t &ioY) const;
	void InsetBy(int32_t inDeltaX, int32_t inDeltaY);

	bool empty() const;
	explicit operator bool() const { return not empty(); }

	// Intersection
	MRect operator&(const MRect &inRegion);
	MRect &operator&=(const MRect &inRegion);

	// Union
	MRect operator|(const MRect &inRegion);
	MRect &operator|=(const MRect &inRegion);

	bool operator==(const MRect &inRect) const
	{
		return x == inRect.x and y == inRect.y and
		       width == inRect.width and height == inRect.height;
	}

	bool operator!=(const MRect &inRect) const
	{
		return x != inRect.x or y != inRect.y or
		       width != inRect.width or height != inRect.height;
	}

	friend std::ostream &operator<<(std::ostream &os, const MRect &r)
	{
		os << '[' << r.x << ',' << r.y << ',' << r.width << ',' << r.height << ']';
		return os;
	}
};

class MRegion
{
  public:
	MRegion();
	MRegion(const MRect &inRect);
	MRegion(const MRegion &inRegion);
	~MRegion();
	MRegion &operator=(const MRegion &inRegion);
	// Intersection
	MRegion operator&(const MRegion &inRegion) const;
	MRegion operator&(const MRect &inRect) const;
	MRegion &operator&=(const MRegion &inRegion);
	MRegion &operator&=(const MRect &inRect);
	// Union
	MRegion operator|(const MRegion &inRegion) const;
	MRegion operator|(const MRect &inRect) const;
	MRegion &operator|=(const MRegion &inRegion);
	MRegion &operator|=(const MRect &inRect);
	// test for empty region
	operator bool() const;
	void OffsetBy(int32_t inX, int32_t inY);
	bool ContainsPoint(int32_t inX, int32_t inY) const;
	MRect GetBounds() const;

  private:
	struct MRegionImpl *mImpl;
};

enum MTriState
{
	eTriStateOn,
	eTriStateOff,
	eTriStateLatent
};

enum MDirection
{
	kDirectionForward = 1,
	kDirectionBackward = -1
};

enum MScrollMessage
{
	kScrollNone,
	kScrollToStart,
	kScrollToEnd,
	kScrollToCaret,
	kScrollToSelection,
	kScrollToThumb,
	kScrollCenterSelection,
	kScrollLineUp,
	kScrollLineDown,
	kScrollPageUp,
	kScrollPageDown,
	kScrollForKiss,
	kScrollReturnAfterKiss,
	kScrollForDiff,
	kScrollToPC
};

enum
{ // modifier keys
	kCmdKey = 1 << 0,
	kShiftKey = 1 << 1,
	kOptionKey = 1 << 2,
	kControlKey = 1 << 3,
	kAlphaLock = 1 << 4,
	kNumPad = 1 << 5,
	kRightShiftKey = 1 << 6,
	kRightOptionKey = 1 << 7,
	kRightControlKey = 1 << 8
};

enum MKeyCode
{ // key codes
	kNullKeyCode = 0,
	kHomeKeyCode = 1,
	kCancelKeyCode = 3,
	kEndKeyCode = 4,
	kInsertKeyCode = 5,
	kBellKeyCode = 7,
	kBackspaceKeyCode = 8,
	kTabKeyCode = 9,
	kLineFeedKeyCode = 10,
	kVerticalTabKeyCode = 11,
	kPageUpKeyCode = 11,
	kFormFeedKeyCode = 12,
	kPageDownKeyCode = 12,
	kReturnKeyCode = 13,
	kFunctionKeyKeyCode = 16,
	kPauseKeyCode = 19,
	kEscapeKeyCode = 27,
	kClearKeyCode = 27,
	kLeftArrowKeyCode = 28,
	kRightArrowKeyCode = 29,
	kUpArrowKeyCode = 30,
	kDownArrowKeyCode = 31,
	kSpaceKeyCode = 32,
	kDeleteKeyCode = 127,
	kDivideKeyCode = 111,
	kMultiplyKeyCode = 106,
	kSubtractKeyCode = 109,
	kNumlockKeyCode = 0x0090,
	kF1KeyCode = 0x0101,
	kF2KeyCode = 0x0102,
	kF3KeyCode = 0x0103,
	kF4KeyCode = 0x0104,
	kF5KeyCode = 0x0105,
	kF6KeyCode = 0x0106,
	kF7KeyCode = 0x0107,
	kF8KeyCode = 0x0108,
	kF9KeyCode = 0x0109,
	kF10KeyCode = 0x010a,
	kF11KeyCode = 0x010b,
	kF12KeyCode = 0x010c,
	kF13KeyCode = 0x010d,
	kF14KeyCode = 0x010e,
	kF15KeyCode = 0x010f,
	kF16KeyCode = 0x0110,
	kF17KeyCode = 0x0111,
	kF18KeyCode = 0x0112,
	kF19KeyCode = 0x0113,
	kF20KeyCode = 0x0114,
	// my own pseudo key codes
	kEnterKeyCode = 0x0201,
};

extern const char kHexChars[];

template <class T>
class value_changer
{
  public:
	value_changer(T &inVariable, const T inTempValue);
	~value_changer();

  private:
	T &mVariable;
	T mValue;
};

template <class T>
inline value_changer<T>::value_changer(T &inVariable, const T inTempValue)
	: mVariable(inVariable)
	, mValue(inVariable)
{
	mVariable = inTempValue;
}

template <class T>
inline value_changer<T>::~value_changer()
{
	mVariable = mValue;
}
