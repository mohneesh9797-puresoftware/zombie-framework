#ifndef OBS_GFX_RENDERINGCONTEXT_HPP
#define OBS_GFX_RENDERINGCONTEXT_HPP

#include "Fwd.hpp"

#include <RenderingKit/RenderingKit.hpp>

#include <framework/datamodel.hpp>

#include <fmt/format.h>

namespace Obs::Gfx {

using RenderingKit::u8string_view;

struct RenderingContext {
    RenderingKit::IRenderingManager& rm;
};

enum class HAlignment {
    left,
    middle,
    right,
};

enum class VAlignment {
    top,
    middle,
    bottom,
};

using Alignment = std::pair<HAlignment, VAlignment>;

inline Alignment leftTop { HAlignment::left, VAlignment::top };

struct RenderingContext2D {
    RenderingContext2D WithAlignment(Alignment alignment) const;
    RenderingContext2D WithFont(RenderingKit::IFontFace& font) const;
    RenderingContext2D WithTextColor(zfw::Byte4 color) const;
    RenderingContext2D WithPosLeftTop(glm::ivec2 pos) const;

    RenderingContext2D DrawTextRow(u8string_view text) const;

    template <typename S, typename... Args>
    RenderingContext2D FormatTextRow(const S& format_str, Args&&... args) {
        fmt::memory_buffer buf;
        fmt::format_to(buf, format_str, args...);
        return DrawTextRow({buf.begin(), buf.size()});
    }

//    glm::ivec2 GetViewSize();

    RenderingContext& ctx;
    Alignment alignment {leftTop};
    RenderingKit::IFontFace* font = nullptr;
    zfw::Byte4 color {0, 0, 0, UINT8_MAX};
    glm::ivec2 pos;
};

}

#endif
