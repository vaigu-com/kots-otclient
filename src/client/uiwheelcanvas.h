/*
 * Copyright (c) 2010-2026 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <array>
#include <framework/ui/uiwidget.h>

// Procedurally draws the Wheel of Destiny colour layer the way the official
// Qt/QML client does: translucent (or additive) tinted annulus-sectors composited
// directly over the shared filigree backdrop, so the engraved detail shows through.
// Replaces the old flat, opaque per-slice "wheel-colors" sprites.
class UIWheelCanvas final : public UIWidget
{
public:
    void drawSelf(DrawPoolType drawPane) override;

    // index is 1..36, matching the WheelButtons / WheelNodes node ids in Lua.
    // "unlocked" draws the faint base band (slice is available for allocation).
    // "fill" (0..1) draws the brighter band growing outward from the inner edge.
    void setUnlocked(int index, bool unlocked);
    void setFill(int index, float fillPercent);
    void clearSlices();

    // false = source-over translucent (matches the QML useBlending=false default),
    // true  = true additive (matches the QML Blend{ mode:"addition" } path).
    void setAdditive(bool additive) { m_additive = additive; }
    bool isAdditive() const { return m_additive; }

protected:
    void onStyleApply(std::string_view styleName, const OTMLNodePtr& styleNode) override;

private:
    struct SliceState
    {
        float fill{ 0.f };
        bool unlocked{ false };
    };

    std::array<SliceState, 37> m_slices{}; // 1-based; index 0 is unused
    bool m_additive{ true };
};
