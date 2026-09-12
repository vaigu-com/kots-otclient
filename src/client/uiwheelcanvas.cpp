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

#include "uiwheelcanvas.h"

#include <algorithm>
#include <cmath>

#include "framework/graphics/drawpoolmanager.h"
#include "framework/otml/otmlnode.h"
#include "framework/util/color.h"

namespace
{
    // Constants ported verbatim from the QML client's SkillWheel.qml / TibiaStyle.qml.
    constexpr float SKILL_WIDTH = 50.f;                 // radial thickness of one ring
    constexpr float LINE_WIDTH = 2.f;                   // TibiaStyle.skillWheelLineWidth
    constexpr float RING_PITCH = SKILL_WIDTH + LINE_WIDTH;
    constexpr float PI_F = 3.14159265358979323846f;
    constexpr float DEG2RAD = PI_F / 180.f;
    constexpr float ARC_STEP_DEG = 3.f;                 // tessellation resolution

    enum Quarter { TL, TR, BL, BR };

    struct SliceGeom
    {
        int radiusIndex;
        float startDeg;
        float endDeg;
        int quarter;
    };

    // Node id (1..36) -> sector geometry, derived so the drawn arcs line up with the
    // shared backdrop_skillwheel.png (authored for the QML client). Angles use screen
    // convention (0deg = east, growing clockwise because y points down), matching
    // QML's Canvas ctx.arc.
    constexpr SliceGeom SLICES[37] = {
        {},                          // 0 unused
        { 4, 195, 255, TL },         // 1
        { 3, 225, 255, TL },         // 2
        { 2, 240, 270, TL },         // 3
        { 2, 270, 300, TR },         // 4
        { 3, 285, 315, TR },         // 5
        { 4, 285, 345, TR },         // 6
        { 3, 195, 225, TL },         // 7
        { 2, 210, 240, TL },         // 8
        { 1, 225, 270, TL },         // 9
        { 1, 270, 315, TR },         // 10
        { 2, 300, 330, TR },         // 11
        { 3, 315, 345, TR },         // 12
        { 2, 180, 210, TL },         // 13
        { 1, 180, 225, TL },         // 14
        { 0, 180, 270, TL },         // 15 (root)
        { 0, 270, 360, TR },         // 16 (root)
        { 1, 315, 360, TR },         // 17
        { 2, 330, 360, TR },         // 18
        { 2, 150, 180, BL },         // 19
        { 1, 135, 180, BL },         // 20
        { 0,  90, 180, BL },         // 21 (root)
        { 0,   0,  90, BR },         // 22 (root)
        { 1,   0,  45, BR },         // 23
        { 2,   0,  30, BR },         // 24
        { 3, 135, 165, BL },         // 25
        { 2, 120, 150, BL },         // 26
        { 1,  90, 135, BL },         // 27
        { 1,  45,  90, BR },         // 28
        { 2,  30,  60, BR },         // 29
        { 3,  15,  45, BR },         // 30
        { 4, 105, 165, BL },         // 31
        { 3, 105, 135, BL },         // 32
        { 2,  90, 120, BL },         // 33
        { 2,  60,  90, BR },         // 34
        { 3,  45,  75, BR },         // 35
        { 4,  15,  75, BR },         // 36
    };

    // Per-quarter ARGB tints from TibiaStyle.qml. {base (faint, available), fill (bright)}.
    Color quarterColor(const int quarter, const bool fill)
    {
        uint32_t v = 0;
        switch (quarter) {
            case TL: v = fill ? 0x6b94c412u : 0x3146b21cu; break;
            case TR: v = fill ? 0x6bef1a38u : 0x31ae1434u; break;
            case BL: v = fill ? 0x6b18cd8fu : 0x31198d40u; break;
            case BR: v = fill ? 0x6be618dau : 0x31841d9eu; break;
            default: v = 0x6bef1a38u; break;
        }
        // v is 0xAARRGGBB; Color(r, g, b, a).
        return { static_cast<int>((v >> 16) & 0xFF),
                 static_cast<int>((v >> 8) & 0xFF),
                 static_cast<int>(v & 0xFF),
                 static_cast<int>((v >> 24) & 0xFF) };
    }

    float innerRadius(const int radiusIndex) { return radiusIndex * RING_PITCH + 1.f; }

    Point polar(const float cx, const float cy, const float radius, const float angleRad)
    {
        return { static_cast<int>(std::lround(cx + radius * std::cos(angleRad))),
                 static_cast<int>(std::lround(cy + radius * std::sin(angleRad))) };
    }

    // Tessellate one annulus sector (inner..outer, startDeg..endDeg) into triangles.
    void drawSector(const float cx, const float cy, const float inner, const float outer,
                    const float startDeg, const float endDeg, const Color& color)
    {
        const int steps = std::max(1, static_cast<int>(std::ceil((endDeg - startDeg) / ARC_STEP_DEG)));
        const float stepRad = (endDeg - startDeg) * DEG2RAD / steps;
        float a0 = startDeg * DEG2RAD;
        for (int k = 0; k < steps; ++k) {
            const float a1 = a0 + stepRad;
            const Point i0 = polar(cx, cy, inner, a0);
            const Point i1 = polar(cx, cy, inner, a1);
            const Point o0 = polar(cx, cy, outer, a0);
            const Point o1 = polar(cx, cy, outer, a1);
            g_drawPool.addFilledTriangle(i0, o0, o1, color);
            g_drawPool.addFilledTriangle(i0, o1, i1, color);
            a0 = a1;
        }
    }
}

void UIWheelCanvas::drawSelf(const DrawPoolType drawPane)
{
    if (drawPane != DrawPoolType::FOREGROUND)
        return;

    const auto& rect = getPaddingRect();
    const float cx = rect.x() + rect.width() / 2.f;
    const float cy = rect.y() + rect.height() / 2.f;

    g_drawPool.setCompositionMode(m_additive ? CompositionMode::ADDITIVE : CompositionMode::NORMAL);

    for (int i = 1; i <= 36; ++i) {
        const SliceState& s = m_slices[i];
        if (s.fill <= 0.f && !s.unlocked)
            continue;

        const SliceGeom& g = SLICES[i];
        const float inner = innerRadius(g.radiusIndex);
        const float outer = inner + SKILL_WIDTH;

        // Bright fill first, growing radially from the inner edge (matches SkillWheel.qml drawSlice).
        if (s.fill > 0.f) {
            const float fillRadius = inner + std::clamp(s.fill, 0.f, 1.f) * (outer - inner);
            drawSector(cx, cy, inner, fillRadius, g.startDeg, g.endDeg, quarterColor(g.quarter, true));
        }

        // Faint base band over the whole ring when the slice is available.
        if (s.unlocked)
            drawSector(cx, cy, inner, outer, g.startDeg, g.endDeg, quarterColor(g.quarter, false));
    }

    g_drawPool.resetCompositionMode();
}

void UIWheelCanvas::setUnlocked(const int index, const bool unlocked)
{
    if (index < 1 || index > 36)
        return;
    m_slices[index].unlocked = unlocked;
}

void UIWheelCanvas::setFill(const int index, const float fillPercent)
{
    if (index < 1 || index > 36)
        return;
    m_slices[index].fill = std::clamp(fillPercent, 0.f, 1.f);
}

void UIWheelCanvas::clearSlices()
{
    for (auto& s : m_slices)
        s = {};
}

void UIWheelCanvas::onStyleApply(const std::string_view styleName, const OTMLNodePtr& styleNode)
{
    UIWidget::onStyleApply(styleName, styleNode);

    for (const auto& node : styleNode->children()) {
        if (node->tag() == "additive")
            setAdditive(node->value<bool>());
    }
}
